#include "platform/windows/hotkey_service.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <sstream>
#include <utility>

namespace lumaspace {
namespace {

UINT nativeModifiers(HotkeyModifier modifiers) noexcept
{
    UINT result = MOD_NOREPEAT;
    if (hasModifier(modifiers, HotkeyModifier::Alt)) {
        result |= MOD_ALT;
    }
    if (hasModifier(modifiers, HotkeyModifier::Control)) {
        result |= MOD_CONTROL;
    }
    if (hasModifier(modifiers, HotkeyModifier::Shift)) {
        result |= MOD_SHIFT;
    }
    if (hasModifier(modifiers, HotkeyModifier::Windows)) {
        result |= MOD_WIN;
    }
    return result;
}

}  // namespace

class HotkeyService::Impl final {
public:
    Impl(ActionRegistry& registry, ModeProvider modeProvider)
        : registry_(registry), modeProvider_(std::move(modeProvider)), ownerThread_(GetCurrentThreadId())
    {
    }

    ~Impl()
    {
        stop();
    }

    HotkeyServiceResult start(std::vector<HotkeyBinding> bindings)
    {
        if (window_ != nullptr) {
            return {HotkeyServiceStatus::AlreadyStarted};
        }
        const HotkeyValidationStatus validation = validateHotkeyBindings(bindings);
        if (validation != HotkeyValidationStatus::Valid) {
            return {HotkeyServiceStatus::InvalidBindings, validation};
        }

        className_ = L"LumaSpace.HotkeyWindow." +
                     std::to_wstring(reinterpret_cast<std::uintptr_t>(this));
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = &Impl::windowProcedure;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = className_.c_str();
        classAtom_ = RegisterClassExW(&windowClass);
        if (classAtom_ == 0) {
            return {HotkeyServiceStatus::WindowCreationFailed,
                    HotkeyValidationStatus::Valid, 0, GetLastError()};
        }

        window_ = CreateWindowExW(0, className_.c_str(), L"", 0, 0, 0, 0, 0,
                                  HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), this);
        if (window_ == nullptr) {
            const DWORD error = GetLastError();
            UnregisterClassW(className_.c_str(), GetModuleHandleW(nullptr));
            classAtom_ = 0;
            return {HotkeyServiceStatus::WindowCreationFailed,
                    HotkeyValidationStatus::Valid, 0, error};
        }

        for (const HotkeyBinding& binding : bindings) {
            if (!RegisterHotKey(window_, binding.registrationId,
                                nativeModifiers(binding.combination.modifiers),
                                binding.combination.virtualKey)) {
                const DWORD error = GetLastError();
                stop();
                return {HotkeyServiceStatus::RegistrationFailed,
                        HotkeyValidationStatus::Valid, binding.registrationId, error};
            }
            registeredIds_.push_back(binding.registrationId);
        }
        bindings_ = std::move(bindings);
        return {HotkeyServiceStatus::Started};
    }

    void stop() noexcept
    {
        if (GetCurrentThreadId() != ownerThread_) {
            return;
        }
        for (const int id : registeredIds_) {
            if (window_ != nullptr) {
                UnregisterHotKey(window_, id);
            }
        }
        registeredIds_.clear();
        bindings_.clear();
        if (window_ != nullptr) {
            DestroyWindow(window_);
            window_ = nullptr;
        }
        if (classAtom_ != 0) {
            UnregisterClassW(className_.c_str(), GetModuleHandleW(nullptr));
            classAtom_ = 0;
        }
        className_.clear();
    }

    bool isRunning() const noexcept
    {
        return window_ != nullptr;
    }

    void* nativeWindowHandle() const noexcept
    {
        return window_;
    }

private:
    static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam,
                                            LPARAM lParam)
    {
        Impl* self = reinterpret_cast<Impl*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            self = static_cast<Impl*>(create->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        if (message == WM_HOTKEY && self != nullptr) {
            self->handleHotkey(static_cast<int>(wParam));
            return 0;
        }
        return DefWindowProcW(window, message, wParam, lParam);
    }

    void handleHotkey(int registrationId)
    {
        const auto iterator = std::find_if(
            bindings_.begin(), bindings_.end(), [registrationId](const HotkeyBinding& binding) {
                return binding.registrationId == registrationId;
            });
        if (iterator == bindings_.end() ||
            !isActionEligible(iterator->activationPolicy, modeProvider_())) {
            return;
        }
        static_cast<void>(registry_.dispatch(iterator->actionId));
    }

    ActionRegistry& registry_;
    ModeProvider modeProvider_;
    DWORD ownerThread_;
    HWND window_ = nullptr;
    ATOM classAtom_ = 0;
    std::wstring className_;
    std::vector<HotkeyBinding> bindings_;
    std::vector<int> registeredIds_;
};

HotkeyService::HotkeyService(ActionRegistry& registry, ModeProvider modeProvider)
    : impl_(std::make_unique<Impl>(registry, std::move(modeProvider)))
{
}

HotkeyService::~HotkeyService() = default;

HotkeyServiceResult HotkeyService::start(std::vector<HotkeyBinding> bindings)
{
    return impl_->start(std::move(bindings));
}

void HotkeyService::stop() noexcept
{
    impl_->stop();
}

bool HotkeyService::isRunning() const noexcept
{
    return impl_->isRunning();
}

void* HotkeyService::nativeWindowHandle() const noexcept
{
    return impl_->nativeWindowHandle();
}

std::string describeHotkeyServiceFailure(const HotkeyServiceResult& result,
                                         const std::vector<HotkeyBinding>& bindings)
{
    std::ostringstream stream;
    if (result.status == HotkeyServiceStatus::InvalidBindings) {
        stream << "Invalid hotkey configuration (validation code "
               << static_cast<int>(result.validation) << ").";
    } else if (result.status == HotkeyServiceStatus::WindowCreationFailed) {
        stream << "Unable to create the hotkey message window (Windows error "
               << result.systemError << ").";
    } else if (result.status == HotkeyServiceStatus::RegistrationFailed) {
        const auto iterator = std::find_if(
            bindings.begin(), bindings.end(), [&result](const HotkeyBinding& binding) {
                return binding.registrationId == result.failedRegistrationId;
            });
        stream << "Unable to register ";
        if (iterator != bindings.end()) {
            stream << hotkeyDisplayName(iterator->combination);
        } else {
            stream << "hotkey ID " << result.failedRegistrationId;
        }
        stream << "; it may be reserved or already in use (Windows error "
               << result.systemError << ").";
    } else {
        stream << "The hotkey service is already running.";
    }
    return stream.str();
}

}  // namespace lumaspace

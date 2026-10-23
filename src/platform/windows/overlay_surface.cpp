#include "platform/windows/overlay_surface.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace lumaspace {
namespace {

constexpr wchar_t kOverlayWindowClass[] = L"LumaSpace.PrimaryOverlay.Proof";

LRESULT CALLBACK overlayWindowProcedure(HWND window, UINT message, WPARAM wParam,
                                        LPARAM lParam)
{
    switch (message) {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATEANDEAT;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

class Win32OverlaySurface final : public IOverlaySurface {
public:
    Win32OverlaySurface() = default;
    ~Win32OverlaySurface() override
    {
        if (window_ != nullptr) {
            DestroyWindow(window_);
        }
        if (classRegistered_) {
            UnregisterClassW(kOverlayWindowClass, GetModuleHandleW(nullptr));
        }
    }

    Win32OverlaySurface(const Win32OverlaySurface&) = delete;
    Win32OverlaySurface& operator=(const Win32OverlaySurface&) = delete;

    [[nodiscard]] bool initialize()
    {
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = overlayWindowProcedure;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = kOverlayWindowClass;
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        const ATOM atom = RegisterClassExW(&windowClass);
        if (atom == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
        classRegistered_ = atom != 0;

        POINT origin{0, 0};
        const HMONITOR monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (monitor == nullptr || !GetMonitorInfoW(monitor, &monitorInfo)) {
            return false;
        }
        const RECT bounds = monitorInfo.rcMonitor;
        const int width = bounds.right - bounds.left;
        const int height = bounds.bottom - bounds.top;
        if (width <= 0 || height <= 0) {
            return false;
        }

        window_ = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW |
                WS_EX_TOPMOST,
            kOverlayWindowClass, L"LumaSpace Overlay Diagnostic", WS_POPUP,
            bounds.left, bounds.top, width, height, nullptr, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (window_ == nullptr || !renderDiagnosticMarker(bounds.left, bounds.top, width, height)) {
            return false;
        }

        ShowWindow(window_, SW_SHOWNOACTIVATE);
        return SetWindowPos(window_, HWND_TOPMOST, bounds.left, bounds.top, width, height,
                            SWP_NOACTIVATE | SWP_SHOWWINDOW) != FALSE;
    }

    [[nodiscard]] void* nativeHandle() const noexcept override
    {
        return window_;
    }

private:
    [[nodiscard]] bool renderDiagnosticMarker(int left, int top, int width, int height)
    {
        BITMAPINFO bitmapInfo{};
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = width;
        bitmapInfo.bmiHeader.biHeight = -height;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        void* pixels = nullptr;
        HDC screen = GetDC(nullptr);
        HDC memory = CreateCompatibleDC(screen);
        HBITMAP bitmap = CreateDIBSection(screen, &bitmapInfo, DIB_RGB_COLORS, &pixels,
                                         nullptr, 0);
        if (screen == nullptr || memory == nullptr || bitmap == nullptr || pixels == nullptr) {
            if (bitmap != nullptr) DeleteObject(bitmap);
            if (memory != nullptr) DeleteDC(memory);
            if (screen != nullptr) ReleaseDC(nullptr, screen);
            return false;
        }

        const std::size_t pixelCount = static_cast<std::size_t>(width) *
                                       static_cast<std::size_t>(height);
        auto* data = static_cast<std::uint32_t*>(pixels);
        std::fill(data, data + pixelCount, 0U);

        constexpr int markerLeft = 32;
        constexpr int markerTop = 32;
        constexpr int markerSize = 92;
        constexpr int thickness = 8;
        constexpr std::uint32_t magenta = 0xE6FF2ABBU;
        constexpr std::uint32_t white = 0xF0F0F0F0U;
        for (int y = markerTop; y < markerTop + markerSize && y < height; ++y) {
            for (int x = markerLeft; x < markerLeft + markerSize && x < width; ++x) {
                const bool border = x < markerLeft + thickness ||
                                    x >= markerLeft + markerSize - thickness ||
                                    y < markerTop + thickness ||
                                    y >= markerTop + markerSize - thickness;
                const bool diagonal = (x - markerLeft) == (y - markerTop) ||
                                      (x - markerLeft) == markerSize - 1 - (y - markerTop);
                if (border || diagonal) {
                    data[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                         static_cast<std::size_t>(x)] = diagonal ? white : magenta;
                }
            }
        }

        HGDIOBJ previous = SelectObject(memory, bitmap);
        POINT destination{left, top};
        SIZE size{width, height};
        POINT source{0, 0};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        const bool updated = UpdateLayeredWindow(window_, screen, &destination, &size,
                                                 memory, &source, 0, &blend,
                                                 ULW_ALPHA) != FALSE;
        SelectObject(memory, previous);
        DeleteObject(bitmap);
        DeleteDC(memory);
        ReleaseDC(nullptr, screen);
        return updated;
    }

    HWND window_ = nullptr;
    bool classRegistered_ = false;
};

}  // namespace

std::unique_ptr<IOverlaySurface> Win32OverlaySurfaceFactory::createPrimaryMonitorSurface()
{
    auto surface = std::make_unique<Win32OverlaySurface>();
    if (!surface->initialize()) {
        return {};
    }
    return surface;
}

Win32ModePlatformAdapter::Win32ModePlatformAdapter(IOverlaySurfaceFactory& surfaceFactory)
    : surfaceFactory_(surfaceFactory)
{
}

ModePlatformResult Win32ModePlatformAdapter::preflight(AppMode target)
{
    return {target == AppMode::Editing ? ModePlatformStatus::NotImplemented
                                       : ModePlatformStatus::Succeeded};
}

ModePlatformResult Win32ModePlatformAdapter::enter(AppMode target)
{
    if (target == AppMode::Editing) {
        return {ModePlatformStatus::NotImplemented};
    }
    if (target == AppMode::Overlay && overlaySurface_ == nullptr) {
        overlaySurface_ = surfaceFactory_.createPrimaryMonitorSurface();
        if (overlaySurface_ == nullptr) {
            return {ModePlatformStatus::Failed};
        }
    }
    return {};
}

ModePlatformResult Win32ModePlatformAdapter::exit(AppMode mode)
{
    if (mode == AppMode::Overlay) {
        overlaySurface_.reset();
    }
    return {};
}

void Win32ModePlatformAdapter::shutdown() noexcept
{
    overlaySurface_.reset();
}

bool Win32ModePlatformAdapter::hasOverlaySurface() const noexcept
{
    return overlaySurface_ != nullptr;
}

void* Win32ModePlatformAdapter::overlayWindowHandle() const noexcept
{
    return overlaySurface_ != nullptr ? overlaySurface_->nativeHandle() : nullptr;
}

}  // namespace lumaspace

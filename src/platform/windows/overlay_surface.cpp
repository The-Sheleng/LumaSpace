#include "platform/windows/overlay_surface.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace lumaspace {
namespace {

LRESULT CALLBACK overlayWindowProcedure(HWND window, UINT message, WPARAM wParam,
                                        LPARAM lParam)
{
    switch (message) {
    case WM_NCHITTEST: return HTTRANSPARENT;
    case WM_MOUSEACTIVATE: return MA_NOACTIVATEANDEAT;
    case WM_ERASEBKGND: return 1;
    default: return DefWindowProcW(window, message, wParam, lParam);
    }
}

class Win32OverlaySurface final : public IOverlaySurface {
public:
    ~Win32OverlaySurface() override
    {
        if (window_ != nullptr) DestroyWindow(window_);
        if (classRegistered_) {
            UnregisterClassW(className_.c_str(), GetModuleHandleW(nullptr));
        }
    }

    [[nodiscard]] bool initialize(const MonitorDescriptor& monitor)
    {
        className_ = L"LumaSpace.Overlay." +
                     std::to_wstring(reinterpret_cast<std::uintptr_t>(this));
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = overlayWindowProcedure;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = className_.c_str();
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        if (RegisterClassExW(&windowClass) == 0) return false;
        classRegistered_ = true;

        const int width = monitor.bounds.right - monitor.bounds.left;
        const int height = monitor.bounds.bottom - monitor.bounds.top;
        const std::wstring title = L"LumaSpace Overlay Diagnostic [" + monitor.id + L"]";
        window_ = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW |
                WS_EX_TOPMOST,
            className_.c_str(), title.c_str(), WS_POPUP, monitor.bounds.left,
            monitor.bounds.top, width, height, nullptr, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (window_ == nullptr ||
            !renderDiagnosticMarker(monitor.bounds.left, monitor.bounds.top, width, height)) {
            return false;
        }
        ShowWindow(window_, SW_SHOWNOACTIVATE);
        return SetWindowPos(window_, HWND_TOPMOST, monitor.bounds.left, monitor.bounds.top,
                            width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW) != FALSE;
    }

    [[nodiscard]] void* nativeHandle() const noexcept override { return window_; }

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
        constexpr std::uint32_t magenta = 0xE6E626A8U;
        constexpr std::uint32_t white = 0xF0E1E1E1U;
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
    std::wstring className_;
    bool classRegistered_ = false;
};

BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM context)
{
    auto* topology = reinterpret_cast<DisplayTopology*>(context);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (GetMonitorInfoW(monitor, &info)) {
        topology->push_back(MonitorDescriptor{
            info.szDevice,
            {info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right,
             info.rcMonitor.bottom},
            (info.dwFlags & MONITORINFOF_PRIMARY) != 0});
    }
    return TRUE;
}

class DisplayChangeWindow final {
public:
    DisplayChangeWindow()
    {
        className_ = L"LumaSpace.DisplayChange." +
                     std::to_wstring(reinterpret_cast<std::uintptr_t>(this));
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = &DisplayChangeWindow::windowProcedure;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = className_.c_str();
        if (RegisterClassExW(&windowClass) == 0) return;
        classRegistered_ = true;
        window_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                  className_.c_str(), L"", WS_POPUP, 0, 0, 0, 0,
                                  nullptr, nullptr, GetModuleHandleW(nullptr), this);
    }

    ~DisplayChangeWindow()
    {
        if (window_ != nullptr) DestroyWindow(window_);
        if (classRegistered_) {
            UnregisterClassW(className_.c_str(), GetModuleHandleW(nullptr));
        }
    }

    void setCallback(IOverlaySurfaceFactory::TopologyChangedCallback callback)
    {
        callback_ = std::move(callback);
    }

private:
    static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam,
                                            LPARAM lParam)
    {
        auto* self = reinterpret_cast<DisplayChangeWindow*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            self = static_cast<DisplayChangeWindow*>(create->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        if (self != nullptr &&
            (message == WM_DISPLAYCHANGE || message == WM_DEVICECHANGE) &&
            self->callback_) {
            self->callback_();
        }
        return DefWindowProcW(window, message, wParam, lParam);
    }

    HWND window_ = nullptr;
    std::wstring className_;
    bool classRegistered_ = false;
    IOverlaySurfaceFactory::TopologyChangedCallback callback_;
};

}  // namespace

bool isValidMonitor(const MonitorDescriptor& monitor) noexcept
{
    return !monitor.id.empty() && monitor.bounds.right > monitor.bounds.left &&
           monitor.bounds.bottom > monitor.bounds.top;
}

DisplayTopology normalizeTopology(DisplayTopology topology)
{
    std::erase_if(topology, [](const MonitorDescriptor& monitor) {
        return !isValidMonitor(monitor);
    });
    std::sort(topology.begin(), topology.end(),
              [](const MonitorDescriptor& left, const MonitorDescriptor& right) {
                  return left.id < right.id;
              });
    topology.erase(std::unique(topology.begin(), topology.end(),
                               [](const MonitorDescriptor& left,
                                  const MonitorDescriptor& right) {
                                   return left.id == right.id;
                               }),
                   topology.end());
    return topology;
}

class Win32OverlaySurfaceFactory::Impl final {
public:
    DisplayChangeWindow displayChangeWindow;
};

Win32OverlaySurfaceFactory::Win32OverlaySurfaceFactory() : impl_(std::make_unique<Impl>()) {}
Win32OverlaySurfaceFactory::~Win32OverlaySurfaceFactory() = default;

DisplayTopology Win32OverlaySurfaceFactory::currentTopology()
{
    DisplayTopology topology;
    EnumDisplayMonitors(nullptr, nullptr, collectMonitor,
                        reinterpret_cast<LPARAM>(&topology));
    return normalizeTopology(std::move(topology));
}

std::unique_ptr<IOverlaySurface> Win32OverlaySurfaceFactory::createSurface(
    const MonitorDescriptor& monitor)
{
    if (!isValidMonitor(monitor)) return {};
    auto surface = std::make_unique<Win32OverlaySurface>();
    if (!surface->initialize(monitor)) return {};
    return surface;
}

void Win32OverlaySurfaceFactory::setTopologyChangedCallback(
    TopologyChangedCallback callback)
{
    impl_->displayChangeWindow.setCallback(std::move(callback));
}

Win32ModePlatformAdapter::Win32ModePlatformAdapter(IOverlaySurfaceFactory& surfaceFactory)
    : surfaceFactory_(surfaceFactory)
{
    surfaceFactory_.setTopologyChangedCallback([this] {
        if (!overlaySurfaces_.empty() && !refreshTopology()) {
            overlaySurfaces_.clear();
            activeTopology_.clear();
            if (overlayFailureHandler_) overlayFailureHandler_();
        }
    });
}

Win32ModePlatformAdapter::~Win32ModePlatformAdapter()
{
    surfaceFactory_.setTopologyChangedCallback({});
}

ModePlatformResult Win32ModePlatformAdapter::preflight(AppMode target)
{
    return {target == AppMode::Editing ? ModePlatformStatus::NotImplemented
                                       : ModePlatformStatus::Succeeded};
}

ModePlatformResult Win32ModePlatformAdapter::enter(AppMode target)
{
    if (target == AppMode::Editing) return {ModePlatformStatus::NotImplemented};
    if (target == AppMode::Overlay && overlaySurfaces_.empty() && !refreshTopology()) {
        return {ModePlatformStatus::Failed};
    }
    return {};
}

ModePlatformResult Win32ModePlatformAdapter::exit(AppMode mode)
{
    if (mode == AppMode::Overlay) {
        overlaySurfaces_.clear();
        activeTopology_.clear();
    }
    return {};
}

void Win32ModePlatformAdapter::shutdown() noexcept
{
    overlaySurfaces_.clear();
    activeTopology_.clear();
    overlayFailureHandler_ = {};
    topologyObserver_ = {};
}

bool Win32ModePlatformAdapter::hasOverlaySurface() const noexcept
{
    return !overlaySurfaces_.empty();
}

std::size_t Win32ModePlatformAdapter::overlaySurfaceCount() const noexcept
{
    return overlaySurfaces_.size();
}

DisplayTopology Win32ModePlatformAdapter::activeTopology() const { return activeTopology_; }

bool Win32ModePlatformAdapter::refreshTopology()
{
    const DisplayTopology topology = normalizeTopology(surfaceFactory_.currentTopology());
    if (topology.empty()) return false;

    std::map<std::wstring, std::unique_ptr<IOverlaySurface>> replacements;
    for (const MonitorDescriptor& monitor : topology) {
        const auto active = std::find_if(
            activeTopology_.begin(), activeTopology_.end(),
            [&monitor](const MonitorDescriptor& existing) { return existing.id == monitor.id; });
        if (active != activeTopology_.end() && *active == monitor) continue;
        auto surface = surfaceFactory_.createSurface(monitor);
        if (surface == nullptr) return false;
        replacements.emplace(monitor.id, std::move(surface));
    }

    std::map<std::wstring, std::unique_ptr<IOverlaySurface>> next;
    for (const MonitorDescriptor& monitor : topology) {
        auto replacement = replacements.find(monitor.id);
        if (replacement != replacements.end()) {
            next.emplace(monitor.id, std::move(replacement->second));
            continue;
        }
        auto existing = overlaySurfaces_.find(monitor.id);
        if (existing == overlaySurfaces_.end()) return false;
        next.emplace(monitor.id, std::move(existing->second));
    }
    overlaySurfaces_ = std::move(next);
    activeTopology_ = topology;
    if (topologyObserver_) topologyObserver_(activeTopology_);
    return true;
}

void Win32ModePlatformAdapter::setOverlayFailureHandler(OverlayFailureHandler handler)
{
    overlayFailureHandler_ = std::move(handler);
}

void Win32ModePlatformAdapter::setTopologyObserver(TopologyObserver observer)
{
    topologyObserver_ = std::move(observer);
}

}  // namespace lumaspace

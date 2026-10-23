#pragma once

#include "app/mode_transition_coordinator.h"

#include <compare>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lumaspace {

struct PhysicalRect {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    auto operator<=>(const PhysicalRect&) const = default;
};

struct MonitorDescriptor {
    std::wstring id;
    PhysicalRect bounds;
    bool primary = false;

    auto operator<=>(const MonitorDescriptor&) const = default;
};

using DisplayTopology = std::vector<MonitorDescriptor>;

[[nodiscard]] bool isValidMonitor(const MonitorDescriptor& monitor) noexcept;
[[nodiscard]] DisplayTopology normalizeTopology(DisplayTopology topology);

class IOverlaySurface {
public:
    virtual ~IOverlaySurface() = default;
    [[nodiscard]] virtual void* nativeHandle() const noexcept = 0;
};

class IOverlaySurfaceFactory {
public:
    using TopologyChangedCallback = std::function<void()>;

    virtual ~IOverlaySurfaceFactory() = default;
    [[nodiscard]] virtual DisplayTopology currentTopology() = 0;
    [[nodiscard]] virtual std::unique_ptr<IOverlaySurface> createSurface(
        const MonitorDescriptor& monitor) = 0;
    virtual void setTopologyChangedCallback(TopologyChangedCallback callback) = 0;
};

class Win32OverlaySurfaceFactory final : public IOverlaySurfaceFactory {
public:
    Win32OverlaySurfaceFactory();
    ~Win32OverlaySurfaceFactory() override;

    [[nodiscard]] DisplayTopology currentTopology() override;
    [[nodiscard]] std::unique_ptr<IOverlaySurface> createSurface(
        const MonitorDescriptor& monitor) override;
    void setTopologyChangedCallback(TopologyChangedCallback callback) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class Win32ModePlatformAdapter final : public IModePlatformAdapter {
public:
    using OverlayFailureHandler = std::function<void()>;
    using TopologyObserver = std::function<void(const DisplayTopology&)>;

    explicit Win32ModePlatformAdapter(IOverlaySurfaceFactory& surfaceFactory);
    ~Win32ModePlatformAdapter() override;

    [[nodiscard]] ModePlatformResult preflight(AppMode target) override;
    [[nodiscard]] ModePlatformResult enter(AppMode target) override;
    [[nodiscard]] ModePlatformResult exit(AppMode mode) override;
    void shutdown() noexcept override;

    [[nodiscard]] bool hasOverlaySurface() const noexcept;
    [[nodiscard]] std::size_t overlaySurfaceCount() const noexcept;
    [[nodiscard]] DisplayTopology activeTopology() const;
    [[nodiscard]] bool refreshTopology();
    void setOverlayFailureHandler(OverlayFailureHandler handler);
    void setTopologyObserver(TopologyObserver observer);

private:
    IOverlaySurfaceFactory& surfaceFactory_;
    std::map<std::wstring, std::unique_ptr<IOverlaySurface>> overlaySurfaces_;
    DisplayTopology activeTopology_;
    OverlayFailureHandler overlayFailureHandler_;
    TopologyObserver topologyObserver_;
};

}  // namespace lumaspace

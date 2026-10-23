#pragma once

#include "app/mode_transition_coordinator.h"

#include <memory>

namespace lumaspace {

class IOverlaySurface {
public:
    virtual ~IOverlaySurface() = default;
    [[nodiscard]] virtual void* nativeHandle() const noexcept = 0;
};

class IOverlaySurfaceFactory {
public:
    virtual ~IOverlaySurfaceFactory() = default;
    [[nodiscard]] virtual std::unique_ptr<IOverlaySurface> createPrimaryMonitorSurface() = 0;
};

class Win32OverlaySurfaceFactory final : public IOverlaySurfaceFactory {
public:
    [[nodiscard]] std::unique_ptr<IOverlaySurface> createPrimaryMonitorSurface() override;
};

class Win32ModePlatformAdapter final : public IModePlatformAdapter {
public:
    explicit Win32ModePlatformAdapter(IOverlaySurfaceFactory& surfaceFactory);

    [[nodiscard]] ModePlatformResult preflight(AppMode target) override;
    [[nodiscard]] ModePlatformResult enter(AppMode target) override;
    [[nodiscard]] ModePlatformResult exit(AppMode mode) override;
    void shutdown() noexcept override;

    [[nodiscard]] bool hasOverlaySurface() const noexcept;
    [[nodiscard]] void* overlayWindowHandle() const noexcept;

private:
    IOverlaySurfaceFactory& surfaceFactory_;
    std::unique_ptr<IOverlaySurface> overlaySurface_;
};

}  // namespace lumaspace

#pragma once

#include <compare>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace lumaspace {

struct WorldPoint {
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const WorldPoint&) const = default;
};

struct VirtualDesktopPhysicalPoint {
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const VirtualDesktopPhysicalPoint&) const = default;
};

struct MonitorLocalPhysicalPoint {
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const MonitorLocalPhysicalPoint&) const = default;
};

struct EditingViewportPoint {
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const EditingViewportPoint&) const = default;
};

struct QtLogicalPoint {
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const QtLogicalPoint&) const = default;
};

struct PhysicalPixelPoint {
    std::int64_t x = 0;
    std::int64_t y = 0;
    auto operator<=>(const PhysicalPixelPoint&) const = default;
};

struct DpiScale {
    std::uint32_t x = 96;
    std::uint32_t y = 96;
    auto operator<=>(const DpiScale&) const = default;
};

struct MonitorCoordinateMapping {
    std::wstring monitorId;
    VirtualDesktopPhysicalPoint desktopOrigin;
    DpiScale dpi;
};

[[nodiscard]] VirtualDesktopPhysicalPoint worldToDesktop(WorldPoint point) noexcept;
[[nodiscard]] WorldPoint desktopToWorld(VirtualDesktopPhysicalPoint point) noexcept;
[[nodiscard]] EditingViewportPoint worldToEditingViewportIdentity(WorldPoint point) noexcept;
[[nodiscard]] WorldPoint editingViewportToWorldIdentity(EditingViewportPoint point) noexcept;
[[nodiscard]] std::optional<PhysicalPixelPoint> roundToPhysicalPixel(
    VirtualDesktopPhysicalPoint point) noexcept;

class CoordinateSpaceRegistry final {
public:
    [[nodiscard]] bool replaceMappings(std::vector<MonitorCoordinateMapping> mappings);
    [[nodiscard]] std::optional<VirtualDesktopPhysicalPoint> monitorToDesktop(
        const std::wstring& monitorId, MonitorLocalPhysicalPoint point) const noexcept;
    [[nodiscard]] std::optional<MonitorLocalPhysicalPoint> desktopToMonitor(
        const std::wstring& monitorId, VirtualDesktopPhysicalPoint point) const noexcept;
    [[nodiscard]] std::optional<MonitorLocalPhysicalPoint> qtLogicalToMonitorPhysical(
        const std::wstring& monitorId, QtLogicalPoint point) const noexcept;
    [[nodiscard]] std::optional<QtLogicalPoint> monitorPhysicalToQtLogical(
        const std::wstring& monitorId, MonitorLocalPhysicalPoint point) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::map<std::wstring, MonitorCoordinateMapping> mappings_;
};

}  // namespace lumaspace

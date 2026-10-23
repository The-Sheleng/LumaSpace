#include "viewport/coordinate_space.h"

#include <cmath>
#include <limits>
#include <utility>

namespace lumaspace {
namespace {

constexpr double kLogicalDpi = 96.0;
constexpr double kMaxExactInteger = 9007199254740991.0;

bool finiteAndRepresentable(double value) noexcept
{
    return std::isfinite(value) && std::abs(value) <= kMaxExactInteger;
}

}  // namespace

VirtualDesktopPhysicalPoint worldToDesktop(WorldPoint point) noexcept
{
    return {point.x, point.y};
}

WorldPoint desktopToWorld(VirtualDesktopPhysicalPoint point) noexcept
{
    return {point.x, point.y};
}

EditingViewportPoint worldToEditingViewportIdentity(WorldPoint point) noexcept
{
    return {point.x, point.y};
}

WorldPoint editingViewportToWorldIdentity(EditingViewportPoint point) noexcept
{
    return {point.x, point.y};
}

std::optional<PhysicalPixelPoint> roundToPhysicalPixel(
    VirtualDesktopPhysicalPoint point) noexcept
{
    if (!finiteAndRepresentable(point.x) || !finiteAndRepresentable(point.y)) {
        return std::nullopt;
    }
    return PhysicalPixelPoint{static_cast<std::int64_t>(std::round(point.x)),
                              static_cast<std::int64_t>(std::round(point.y))};
}

bool CoordinateSpaceRegistry::replaceMappings(
    std::vector<MonitorCoordinateMapping> mappings)
{
    std::map<std::wstring, MonitorCoordinateMapping> replacement;
    for (MonitorCoordinateMapping& mapping : mappings) {
        if (mapping.monitorId.empty() || mapping.dpi.x == 0 || mapping.dpi.y == 0 ||
            !finiteAndRepresentable(mapping.desktopOrigin.x) ||
            !finiteAndRepresentable(mapping.desktopOrigin.y)) {
            return false;
        }
        const std::wstring id = mapping.monitorId;
        if (!replacement.emplace(id, std::move(mapping)).second) return false;
    }
    mappings_ = std::move(replacement);
    return true;
}

std::optional<VirtualDesktopPhysicalPoint> CoordinateSpaceRegistry::monitorToDesktop(
    const std::wstring& monitorId, MonitorLocalPhysicalPoint point) const noexcept
{
    const auto mapping = mappings_.find(monitorId);
    if (mapping == mappings_.end()) return std::nullopt;
    const double x = mapping->second.desktopOrigin.x + point.x;
    const double y = mapping->second.desktopOrigin.y + point.y;
    if (!finiteAndRepresentable(x) || !finiteAndRepresentable(y)) return std::nullopt;
    return VirtualDesktopPhysicalPoint{x, y};
}

std::optional<MonitorLocalPhysicalPoint> CoordinateSpaceRegistry::desktopToMonitor(
    const std::wstring& monitorId, VirtualDesktopPhysicalPoint point) const noexcept
{
    const auto mapping = mappings_.find(monitorId);
    if (mapping == mappings_.end()) return std::nullopt;
    const double x = point.x - mapping->second.desktopOrigin.x;
    const double y = point.y - mapping->second.desktopOrigin.y;
    if (!finiteAndRepresentable(x) || !finiteAndRepresentable(y)) return std::nullopt;
    return MonitorLocalPhysicalPoint{x, y};
}

std::optional<MonitorLocalPhysicalPoint>
CoordinateSpaceRegistry::qtLogicalToMonitorPhysical(const std::wstring& monitorId,
                                                    QtLogicalPoint point) const noexcept
{
    const auto mapping = mappings_.find(monitorId);
    if (mapping == mappings_.end() || !std::isfinite(point.x) || !std::isfinite(point.y)) {
        return std::nullopt;
    }
    const double x = point.x * static_cast<double>(mapping->second.dpi.x) / kLogicalDpi;
    const double y = point.y * static_cast<double>(mapping->second.dpi.y) / kLogicalDpi;
    if (!finiteAndRepresentable(x) || !finiteAndRepresentable(y)) return std::nullopt;
    return MonitorLocalPhysicalPoint{x, y};
}

std::optional<QtLogicalPoint> CoordinateSpaceRegistry::monitorPhysicalToQtLogical(
    const std::wstring& monitorId, MonitorLocalPhysicalPoint point) const noexcept
{
    const auto mapping = mappings_.find(monitorId);
    if (mapping == mappings_.end() || !std::isfinite(point.x) || !std::isfinite(point.y)) {
        return std::nullopt;
    }
    return QtLogicalPoint{
        point.x * kLogicalDpi / static_cast<double>(mapping->second.dpi.x),
        point.y * kLogicalDpi / static_cast<double>(mapping->second.dpi.y)};
}

std::size_t CoordinateSpaceRegistry::size() const noexcept { return mappings_.size(); }

}  // namespace lumaspace

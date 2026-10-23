#pragma once

#include <QString>
#include <QStringView>

namespace lumaspace {

inline constexpr QStringView kProductName = u"LumaSpace";
inline constexpr QStringView kApplicationIdentifier = u"com.lumaspace.app";
inline constexpr QStringView kVersion = u"0.1.0";

[[nodiscard]] QString productDisplayName();

}  // namespace lumaspace

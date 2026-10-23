#include "viewport/coordinate_space.h"

#include <QTest>

#include <cmath>
#include <limits>

class CoordinateSpaceTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void worldDesktopAndViewportIdentityRoundTrip()
    {
        const lumaspace::WorldPoint world{-1920.25, 1080.75};
        QCOMPARE(lumaspace::desktopToWorld(lumaspace::worldToDesktop(world)), world);
        QCOMPARE(lumaspace::editingViewportToWorldIdentity(
                     lumaspace::worldToEditingViewportIdentity(world)),
                 world);
    }

    void signedMonitorOffsetsAndEdgePointsRoundTrip()
    {
        lumaspace::CoordinateSpaceRegistry registry;
        QVERIFY(registry.replaceMappings({
            {L"left", {-2560.0, -240.0}, {144, 144}},
            {L"primary", {0.0, 0.0}, {96, 96}},
        }));
        const lumaspace::MonitorLocalPhysicalPoint local{2559.5, 1439.5};
        const auto desktop = registry.monitorToDesktop(L"left", local);
        QVERIFY(desktop.has_value());
        QCOMPARE(*desktop, (lumaspace::VirtualDesktopPhysicalPoint{-0.5, 1199.5}));
        QCOMPARE(*registry.desktopToMonitor(L"left", *desktop), local);
        QVERIFY(!registry.monitorToDesktop(L"missing", local).has_value());
    }

    void mixedDpiLogicalPhysicalRoundTrips_data()
    {
        QTest::addColumn<unsigned int>("dpi");
        QTest::addColumn<double>("physical");
        QTest::newRow("100-percent") << 96U << 80.0;
        QTest::newRow("125-percent") << 120U << 100.0;
        QTest::newRow("150-percent") << 144U << 120.0;
        QTest::newRow("200-percent") << 192U << 160.0;
    }

    void mixedDpiLogicalPhysicalRoundTrips()
    {
        QFETCH(unsigned int, dpi);
        QFETCH(double, physical);
        lumaspace::CoordinateSpaceRegistry registry;
        QVERIFY(registry.replaceMappings({{L"display", {0.0, 0.0}, {dpi, dpi}}}));
        const auto converted = registry.qtLogicalToMonitorPhysical(
            L"display", lumaspace::QtLogicalPoint{80.0, 80.0});
        QVERIFY(converted.has_value());
        QCOMPARE(converted->x, physical);
        QCOMPARE(*registry.monitorPhysicalToQtLogical(L"display", *converted),
                 (lumaspace::QtLogicalPoint{80.0, 80.0}));
    }

    void roundingUsesNearestPixelWithHalfAwayFromZero()
    {
        QCOMPARE(*lumaspace::roundToPhysicalPixel({10.49, -10.49}),
                 (lumaspace::PhysicalPixelPoint{10, -10}));
        QCOMPARE(*lumaspace::roundToPhysicalPixel({10.5, -10.5}),
                 (lumaspace::PhysicalPixelPoint{11, -11}));
    }

    void invalidAndOverflowInputsAreRejectedAtomically()
    {
        lumaspace::CoordinateSpaceRegistry registry;
        QVERIFY(registry.replaceMappings({{L"good", {0.0, 0.0}, {96, 96}}}));
        QVERIFY(!registry.replaceMappings({{L"bad", {0.0, 0.0}, {0, 96}}}));
        QCOMPARE(registry.size(), std::size_t{1});
        QVERIFY(!lumaspace::roundToPhysicalPixel(
                     {std::numeric_limits<double>::infinity(), 0.0})
                     .has_value());
        QVERIFY(!lumaspace::roundToPhysicalPixel({1.0e30, 0.0}).has_value());
        QVERIFY(!registry.monitorToDesktop(
                     L"good", {std::numeric_limits<double>::quiet_NaN(), 0.0})
                     .has_value());
    }

    void mappingsAreReplacedByStableMonitorId()
    {
        lumaspace::CoordinateSpaceRegistry registry;
        QVERIFY(registry.replaceMappings({{L"a", {-100.0, 0.0}, {96, 96}},
                                          {L"b", {0.0, 0.0}, {120, 120}}}));
        const lumaspace::WorldPoint world{42.0, 73.0};
        QVERIFY(registry.replaceMappings({{L"b", {-500.0, 20.0}, {144, 144}}}));
        QCOMPARE(lumaspace::desktopToWorld(lumaspace::worldToDesktop(world)), world);
        QCOMPARE(registry.size(), std::size_t{1});
        QVERIFY(!registry.monitorToDesktop(L"a", {0.0, 0.0}).has_value());
        QCOMPARE(*registry.monitorToDesktop(L"b", {0.0, 0.0}),
                 (lumaspace::VirtualDesktopPhysicalPoint{-500.0, 20.0}));
    }
};

QTEST_APPLESS_MAIN(CoordinateSpaceTest)

#include "coordinate_space_test.moc"

#include "platform/windows/overlay_surface.h"

#include <QTest>

#include <memory>
#include <string>
#include <vector>

namespace {

lumaspace::MonitorDescriptor monitor(std::wstring id, int left, int top, int right,
                                     int bottom, bool primary = false)
{
    return {std::move(id), {left, top, right, bottom}, primary};
}

class FakeOverlaySurface final : public lumaspace::IOverlaySurface {
public:
    explicit FakeOverlaySurface(int& destructions) : destructions_(destructions) {}
    ~FakeOverlaySurface() override { ++destructions_; }
    void* nativeHandle() const noexcept override { return reinterpret_cast<void*>(1); }

private:
    int& destructions_;
};

class FakeOverlayFactory final : public lumaspace::IOverlaySurfaceFactory {
public:
    lumaspace::DisplayTopology topology{monitor(L"primary", 0, 0, 1920, 1080, true)};
    std::wstring failingId;
    int creations = 0;
    int destructions = 0;
    std::vector<lumaspace::MonitorDescriptor> requested;
    TopologyChangedCallback callback;

    lumaspace::DisplayTopology currentTopology() override { return topology; }

    std::unique_ptr<lumaspace::IOverlaySurface> createSurface(
        const lumaspace::MonitorDescriptor& descriptor) override
    {
        ++creations;
        requested.push_back(descriptor);
        if (descriptor.id == failingId) return {};
        return std::make_unique<FakeOverlaySurface>(destructions);
    }

    void setTopologyChangedCallback(TopologyChangedCallback value) override
    {
        callback = std::move(value);
    }

    void notify() { if (callback) callback(); }
};

}  // namespace

class OverlaySurfaceTest final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void repeatedEnterExitUsesRaii()
    {
        FakeOverlayFactory factory;
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        QCOMPARE(adapter.enter(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        QCOMPARE(adapter.enter(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        QCOMPARE(factory.creations, 1);
        QCOMPARE(adapter.overlaySurfaceCount(), std::size_t{1});
        QCOMPARE(adapter.exit(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        QCOMPARE(factory.destructions, 1);
        QCOMPARE(adapter.enter(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        adapter.shutdown();
        QCOMPARE(factory.creations, 2);
        QCOMPARE(factory.destructions, 2);
    }

    void syntheticTopologiesPreserveSignedPhysicalBounds()
    {
        FakeOverlayFactory factory;
        factory.topology = {
            monitor(L"right", 0, 0, 1920, 1080, true),
            monitor(L"left", -1280, 100, 0, 1124),
            monitor(L"above", 0, -900, 1600, 0),
        };
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        QVERIFY(adapter.refreshTopology());
        QCOMPARE(adapter.overlaySurfaceCount(), std::size_t{3});
        QCOMPARE(factory.creations, 3);
        QVERIFY(adapter.refreshTopology());
        QCOMPARE(factory.creations, 3);
        QCOMPARE(adapter.activeTopology()[0].id, std::wstring{L"above"});
        QCOMPARE(adapter.activeTopology()[0].bounds.top, -900);
        QCOMPARE(adapter.activeTopology()[1].bounds.left, -1280);
    }

    void primarySwapRecreatesOnlyChangedDescriptors()
    {
        FakeOverlayFactory factory;
        factory.topology = {monitor(L"a", -1920, 0, 0, 1080),
                            monitor(L"b", 0, 0, 1920, 1080, true)};
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        QVERIFY(adapter.refreshTopology());
        QCOMPARE(factory.creations, 2);
        factory.topology = {monitor(L"a", -1920, 0, 0, 1080, true),
                            monitor(L"b", 0, 0, 1920, 1080)};
        QVERIFY(adapter.refreshTopology());
        QCOMPARE(factory.creations, 4);
        QCOMPARE(factory.destructions, 2);
        QVERIFY(adapter.activeTopology()[0].primary);
    }

    void disconnectDestroysOnlyRemovedSurface()
    {
        FakeOverlayFactory factory;
        factory.topology = {monitor(L"left", -1280, 0, 0, 1024),
                            monitor(L"main", 0, 0, 1920, 1080, true)};
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        QVERIFY(adapter.refreshTopology());
        factory.topology = {monitor(L"main", 0, 0, 1920, 1080, true)};
        factory.notify();
        QCOMPARE(adapter.overlaySurfaceCount(), std::size_t{1});
        QCOMPARE(factory.destructions, 1);
        QCOMPARE(adapter.activeTopology().front().id, std::wstring{L"main"});
    }

    void zeroDisplaysAndCreationFailureFailSafe()
    {
        FakeOverlayFactory factory;
        factory.topology.clear();
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        lumaspace::AppModeController controller;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        const auto empty = coordinator.requestTransition(
            lumaspace::AppMode::Overlay, lumaspace::TransitionSource::Test);
        QCOMPARE(empty.reason, lumaspace::CoordinatedTransitionReason::EnterFailed);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);

        factory.topology = {monitor(L"main", 0, 0, 1920, 1080, true)};
        factory.failingId = L"main";
        const auto failed = coordinator.requestTransition(
            lumaspace::AppMode::Overlay, lumaspace::TransitionSource::Test);
        QCOMPARE(failed.reason, lumaspace::CoordinatedTransitionReason::EnterFailed);
        QVERIFY(!adapter.hasOverlaySurface());
    }

    void hotplugFailureRequestsSafeMode()
    {
        FakeOverlayFactory factory;
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        lumaspace::AppModeController controller;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        QVERIFY(coordinator
                    .requestTransition(lumaspace::AppMode::Overlay,
                                       lumaspace::TransitionSource::Test)
                    .succeeded());
        int failures = 0;
        adapter.setOverlayFailureHandler([&] {
            ++failures;
            static_cast<void>(coordinator.requestTransition(
                lumaspace::AppMode::Disabled, lumaspace::TransitionSource::Lifecycle));
        });
        factory.topology.push_back(monitor(L"broken", 1920, 0, 3840, 1080));
        factory.failingId = L"broken";
        factory.notify();
        QCOMPARE(failures, 1);
        QVERIFY(!adapter.hasOverlaySurface());
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QCOMPARE(factory.destructions, 1);
    }

    void normalizationRejectsInvalidAndDuplicateDisplays()
    {
        auto result = lumaspace::normalizeTopology({
            monitor(L"b", 0, 0, 10, 10), monitor(L"", 0, 0, 10, 10),
            monitor(L"a", 0, 0, 0, 10), monitor(L"b", 10, 0, 20, 10),
        });
        QCOMPARE(result.size(), std::size_t{1});
        QCOMPARE(result.front().id, std::wstring{L"b"});
    }
};

QTEST_APPLESS_MAIN(OverlaySurfaceTest)

#include "overlay_surface_test.moc"

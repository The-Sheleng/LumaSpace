#include "platform/windows/overlay_surface.h"

#include <QTest>

#include <memory>

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
    bool fail = false;
    int creations = 0;
    int destructions = 0;

    std::unique_ptr<lumaspace::IOverlaySurface> createPrimaryMonitorSurface() override
    {
        ++creations;
        if (fail) return {};
        return std::make_unique<FakeOverlaySurface>(destructions);
    }
};

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
        QVERIFY(adapter.hasOverlaySurface());
        QCOMPARE(adapter.exit(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        QVERIFY(!adapter.hasOverlaySurface());
        QCOMPARE(factory.destructions, 1);
        QCOMPARE(adapter.enter(lumaspace::AppMode::Overlay).status,
                 lumaspace::ModePlatformStatus::Succeeded);
        adapter.shutdown();
        QCOMPARE(factory.creations, 2);
        QCOMPARE(factory.destructions, 2);
    }

    void factoryFailurePreventsOverlayCommit()
    {
        FakeOverlayFactory factory;
        factory.fail = true;
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        lumaspace::AppModeController controller;
        lumaspace::ModeTransitionCoordinator coordinator(controller, adapter);
        const auto result = coordinator.requestTransition(
            lumaspace::AppMode::Overlay, lumaspace::TransitionSource::Test);
        QCOMPARE(result.status, lumaspace::CoordinatedTransitionStatus::Failed);
        QCOMPARE(result.reason, lumaspace::CoordinatedTransitionReason::EnterFailed);
        QCOMPARE(controller.currentMode(), lumaspace::AppMode::Disabled);
        QVERIFY(!adapter.hasOverlaySurface());
    }

    void editingRemainsUnavailable()
    {
        FakeOverlayFactory factory;
        lumaspace::Win32ModePlatformAdapter adapter(factory);
        QCOMPARE(adapter.preflight(lumaspace::AppMode::Editing).status,
                 lumaspace::ModePlatformStatus::NotImplemented);
        QCOMPARE(factory.creations, 0);
    }
};

QTEST_APPLESS_MAIN(OverlaySurfaceTest)

#include "overlay_surface_test.moc"

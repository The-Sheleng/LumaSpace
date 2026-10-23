#include "actions/mode_actions.h"
#include "app/app_mode.h"
#include "app/logging.h"
#include "app/product_info.h"
#include "app/single_instance.h"
#include "platform/windows/hotkey_service.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QIcon>
#include <QLabel>
#include <QLoggingCategory>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>

namespace lumaspace {
namespace {

QIcon createTrayIcon()
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(93, 78, 180));
    painter.drawEllipse(2, 2, 60, 60);
    painter.setPen(QPen(Qt::white, 7, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(22, 15, 22, 46);
    painter.drawLine(22, 46, 45, 46);
    return QIcon(pixmap);
}

QWidget* createSettingsWindow()
{
    auto* window = new QWidget;
    window->setAttribute(Qt::WA_DeleteOnClose, false);
    window->setWindowTitle(QStringLiteral("LumaSpace Settings"));
    window->resize(420, 180);

    auto* layout = new QVBoxLayout(window);
    auto* label = new QLabel(QStringLiteral("LumaSpace settings will be added in a later milestone."));
    label->setWordWrap(true);
    auto* closeButton = new QPushButton(QStringLiteral("Close"));
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(closeButton);
    QObject::connect(closeButton, &QPushButton::clicked, window, &QWidget::close);
    return window;
}

}  // namespace
}  // namespace lumaspace

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName(lumaspace::kProductName.toString());
    QCoreApplication::setApplicationName(lumaspace::kProductName.toString());
    QCoreApplication::setApplicationVersion(lumaspace::kVersion.toString());
    qSetMessagePattern(QStringLiteral("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}"));

    QApplication application(argc, argv);
    application.setQuitOnLastWindowClosed(false);
    application.setWindowIcon(lumaspace::createTrayIcon());

    lumaspace::SingleInstance singleInstance;
    const auto lockResult = singleInstance.acquire();
    if (lockResult == lumaspace::SingleInstance::Result::AlreadyRunning) {
        qInfo().noquote() << singleInstance.errorMessage();
        return 0;
    }
    if (lockResult == lumaspace::SingleInstance::Result::Error) {
        qCritical().noquote() << singleInstance.errorMessage();
        QMessageBox::critical(nullptr, lumaspace::productDisplayName(),
                              singleInstance.errorMessage());
        return 1;
    }

    lumaspace::LoggingSession logging;
    if (!logging.initialize()) {
        QMessageBox::warning(nullptr, lumaspace::productDisplayName(),
                             QStringLiteral("LumaSpace could not initialize logging.\n\n%1")
                                 .arg(logging.errorMessage()));
    }
    qInfo() << "Starting" << lumaspace::productDisplayName()
            << lumaspace::kVersion.toString() << "Log:" << logging.logFilePath();

    lumaspace::AppModeController modeController([](const lumaspace::ModeTransition& transition) {
        qInfo() << "Application mode changed from"
                << lumaspace::appModeName(transition.from).data() << "to"
                << lumaspace::appModeName(transition.to).data();
    });
    lumaspace::UnsupportedModePlatformAdapter modePlatformAdapter;
    lumaspace::ModeTransitionCoordinator modeCoordinator(
        modeController, modePlatformAdapter,
        [](const lumaspace::CoordinatedTransitionResult& result) {
            qWarning() << "Application mode request rejected. Requested:"
                       << lumaspace::appModeName(result.requested).data()
                       << "reason code:" << static_cast<int>(result.reason);
        });
    lumaspace::ActionRegistry actionRegistry;
    if (!lumaspace::registerModeActions(actionRegistry, modeCoordinator)) {
        qCritical() << "Unable to register the built-in mode actions.";
        QMessageBox::critical(nullptr, lumaspace::productDisplayName(),
                              QStringLiteral("LumaSpace could not register its built-in actions."));
        return 1;
    }
    const std::vector<lumaspace::HotkeyBinding> hotkeyBindings =
        lumaspace::defaultModeHotkeys();
    lumaspace::HotkeyService hotkeyService(
        actionRegistry, [&modeCoordinator] { return modeCoordinator.currentMode(); });
    const lumaspace::HotkeyServiceResult hotkeyResult = hotkeyService.start(hotkeyBindings);
    if (!hotkeyResult.succeeded()) {
        const QString message = QString::fromStdString(
            lumaspace::describeHotkeyServiceFailure(hotkeyResult, hotkeyBindings));
        qCritical().noquote() << message;
        QMessageBox::critical(nullptr, lumaspace::productDisplayName(), message);
        return 1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        const QString message = QStringLiteral(
            "The Windows notification area is unavailable. LumaSpace cannot start.");
        qCritical().noquote() << message;
        QMessageBox::critical(nullptr, lumaspace::productDisplayName(), message);
        return 1;
    }

    std::unique_ptr<QWidget> settingsWindow(lumaspace::createSettingsWindow());
    QMenu trayMenu;
    QAction* showSettingsAction = trayMenu.addAction(QStringLiteral("Show Settings"));
    trayMenu.addSeparator();
    QAction* exitAction = trayMenu.addAction(QStringLiteral("Exit"));

    QSystemTrayIcon trayIcon(application.windowIcon());
    trayIcon.setToolTip(lumaspace::productDisplayName());
    trayIcon.setContextMenu(&trayMenu);

    QObject::connect(showSettingsAction, &QAction::triggered, settingsWindow.get(), [&settingsWindow] {
        settingsWindow->show();
        settingsWindow->raise();
        settingsWindow->activateWindow();
    });
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, settingsWindow.get(),
                     [&settingsWindow](QSystemTrayIcon::ActivationReason reason) {
                         if (reason == QSystemTrayIcon::DoubleClick) {
                             settingsWindow->show();
                             settingsWindow->raise();
                             settingsWindow->activateWindow();
                         }
                     });
    QObject::connect(exitAction, &QAction::triggered, &application, [&application, &trayIcon] {
        trayIcon.hide();
        qInfo() << "Exiting LumaSpace";
        application.quit();
    });

    trayIcon.show();
    const int exitCode = application.exec();
    hotkeyService.stop();
    modeCoordinator.shutdown();
    return exitCode;
}

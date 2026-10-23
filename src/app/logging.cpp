#include "app/logging.h"

#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>

#include <cstdio>

namespace lumaspace {

LoggingSession* LoggingSession::activeSession_ = nullptr;

LoggingSession::LoggingSession() = default;

LoggingSession::~LoggingSession()
{
    if (activeSession_ == this) {
        qInstallMessageHandler(previousHandler_);
        activeSession_ = nullptr;
    }
    if (logFile_.isOpen()) {
        logFile_.flush();
        logFile_.close();
    }
}

bool LoggingSession::initialize()
{
    if (activeSession_ != nullptr) {
        errorMessage_ = QStringLiteral("Logging has already been initialized.");
        return false;
    }

    const QString directoryPath =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (directoryPath.isEmpty() || !QDir().mkpath(directoryPath)) {
        errorMessage_ = QStringLiteral("Unable to create the application log directory.");
        return false;
    }

    logFile_.setFileName(QDir(directoryPath).filePath(QStringLiteral("LumaSpace.log")));
    if (!logFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        errorMessage_ = QStringLiteral("Unable to open the log file: %1")
                            .arg(logFile_.errorString());
        return false;
    }

    activeSession_ = this;
    previousHandler_ = qInstallMessageHandler(&LoggingSession::messageHandler);
    errorMessage_.clear();
    return true;
}

QString LoggingSession::errorMessage() const
{
    return errorMessage_;
}

QString LoggingSession::logFilePath() const
{
    return logFile_.fileName();
}

void LoggingSession::messageHandler(QtMsgType type, const QMessageLogContext& context,
                                    const QString& message)
{
    if (activeSession_ != nullptr) {
        activeSession_->writeMessage(type, context, message);
    }
}

void LoggingSession::writeMessage(QtMsgType type, const QMessageLogContext& context,
                                  const QString& message)
{
    const QByteArray formatted = qFormatLogMessage(type, context, message).toLocal8Bit();
    const QMutexLocker locker(&mutex_);
    logFile_.write(formatted);
    logFile_.write("\n");
    logFile_.flush();
    std::fwrite(formatted.constData(), 1, static_cast<std::size_t>(formatted.size()), stderr);
    std::fwrite("\n", 1, 1, stderr);
    std::fflush(stderr);
}

}  // namespace lumaspace

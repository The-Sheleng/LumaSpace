#pragma once

#include <QFile>
#include <QMessageLogContext>
#include <QMutex>
#include <QString>
#include <QtLogging>

namespace lumaspace {

class LoggingSession final {
public:
    LoggingSession();
    ~LoggingSession();

    LoggingSession(const LoggingSession&) = delete;
    LoggingSession& operator=(const LoggingSession&) = delete;

    [[nodiscard]] bool initialize();
    [[nodiscard]] QString errorMessage() const;
    [[nodiscard]] QString logFilePath() const;

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext& context,
                               const QString& message);
    void writeMessage(QtMsgType type, const QMessageLogContext& context,
                      const QString& message);

    // Qt message handling is process-wide, so the active RAII-owned session is
    // the single justified global lifecycle reference. Writes are synchronized.
    static LoggingSession* activeSession_;

    QFile logFile_;
    QMutex mutex_;
    QtMessageHandler previousHandler_ = nullptr;
    QString errorMessage_;
};

}  // namespace lumaspace

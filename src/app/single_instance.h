#pragma once

#include <QLockFile>
#include <QString>

#include <memory>

namespace lumaspace {

class SingleInstance final {
public:
    enum class Result {
        Acquired,
        AlreadyRunning,
        Error,
    };

    SingleInstance();
    ~SingleInstance();

    SingleInstance(const SingleInstance&) = delete;
    SingleInstance& operator=(const SingleInstance&) = delete;

    [[nodiscard]] Result acquire();
    [[nodiscard]] QString errorMessage() const;

private:
    std::unique_ptr<QLockFile> lockFile_;
    QString lockPath_;
    QString errorMessage_;
};

}  // namespace lumaspace

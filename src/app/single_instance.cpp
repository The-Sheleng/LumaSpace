#include "app/single_instance.h"

#include "app/product_info.h"

#include <QDir>
#include <QStandardPaths>

namespace lumaspace {

SingleInstance::SingleInstance()
{
    lockPath_ = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                    .filePath(kApplicationIdentifier.toString() + QStringLiteral(".lock"));
    lockFile_ = std::make_unique<QLockFile>(lockPath_);
}

SingleInstance::~SingleInstance() = default;

SingleInstance::Result SingleInstance::acquire()
{
    if (lockFile_->tryLock(100)) {
        errorMessage_.clear();
        return Result::Acquired;
    }

    if (lockFile_->error() == QLockFile::LockFailedError) {
        errorMessage_ = QStringLiteral("Another LumaSpace instance is already running.");
        return Result::AlreadyRunning;
    }

    errorMessage_ = QStringLiteral("Unable to create the single-instance lock: %1")
                        .arg(lockPath_);
    return Result::Error;
}

QString SingleInstance::errorMessage() const
{
    return errorMessage_;
}

}  // namespace lumaspace

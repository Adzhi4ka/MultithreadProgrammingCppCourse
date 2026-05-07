#pragma once

#include <functional>
#include <vector>

#include "domain/services/remote-service-base.h"
#include "infrastructure/repositories/file-repository.h"

namespace client::domain::services {

class FileWorkspaceService : public RemoteServiceBase {

        using FileVersion = domain::models::FileVersion;
        using FileLock = domain::models::FileLock;
        infrastructure::repositories::FileRepository& m_fileRepository;

    public:

        FileWorkspaceService(application::NetworkWorker& networkWorker, QObject& internalContext, QObject& uiContext,
                             infrastructure::repositories::FileRepository& fileRepository) noexcept;

        void downloadCurrent(qint64 fileId, std::function<void(ApiResult<QByteArray>)> callback);
        void uploadCurrent(qint64 fileId, QByteArray content, std::function<void(ApiResult<FileVersion>)> callback);
        void downloadVersion(qint64 versionId, std::function<void(ApiResult<QByteArray>)> callback);

        void acquireLock(qint64 fileId, qint64 lockDurationSec, std::function<void(ApiResult<FileLock>)> callback);
        void renewLock(qint64 fileId, qint64 lockToken, qint64 lockDurationSec,
                       std::function<void(ApiResult<void>)> callback);
        void releaseLock(qint64 fileId, qint64 lockToken, std::function<void(ApiResult<void>)> callback);

        void loadVersions(qint64 fileId, std::function<void(ApiResult<std::vector<FileVersion>>)> callback);
};

}  // namespace client::domain::services

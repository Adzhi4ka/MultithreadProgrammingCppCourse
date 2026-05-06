#pragma once

#include "domain/services/remote-service-base.h"
#include "infrastructure/repositories/file-repository.h"

#include <functional>
#include <memory>
#include <vector>

namespace client::domain::services {

    class FileCatalogService : public RemoteServiceBase {

            using RemoteFile = domain::models::RemoteFile;
            infrastructure::repositories::FileRepository& m_fileRepository;

        public:

            FileCatalogService(application::NetworkWorker& networkWorker,
                            QObject& internalContext,
                            QObject& uiContext,
                            infrastructure::repositories::FileRepository& fileRepository) noexcept;

            void refreshFiles(qint64 currentUserId,
                            std::function<void(ApiResult<std::vector<RemoteFile>>)> callback);
            void createFile(QString logicalName,
                            quint32 maxVersionCount,
                            std::function<void(ApiResult<RemoteFile>)> callback);
            void renameFile(qint64 fileId,
                            QString newLogicalName,
                            std::function<void(ApiResult<RemoteFile>)> callback);
            void deleteFile(qint64 fileId, std::function<void(ApiResult<void>)> callback);

        private:
            struct FileHydrationState;

            void hydrateFiles(qint64 currentUserId,
                            ApiResult<std::vector<RemoteFile>> result,
                            std::shared_ptr<std::function<void(ApiResult<std::vector<RemoteFile>>)>> callback);
            void requestAclHydration(qint64 fileId,
                                    qint64 currentUserId,
                                    std::shared_ptr<FileHydrationState> state);
            void requestLockHydration(qint64 fileId,
                                    std::shared_ptr<FileHydrationState> state);
            void finishFileHydration(const std::shared_ptr<FileHydrationState>& state);
            static RemoteFile* findFile(std::vector<RemoteFile>& files,
                                                        qint64 fileId);

    };

}

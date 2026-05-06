#include "file-workspace-service.h"

#include <memory>
#include <utility>

namespace client::domain::services {

    FileWorkspaceService::FileWorkspaceService(application::NetworkWorker& networkWorker,
                                               QObject& internalContext,
                                               QObject& uiContext,
                                               infrastructure::repositories::FileRepository& fileRepository) noexcept
        : RemoteServiceBase(networkWorker, internalContext, uiContext),
          m_fileRepository(fileRepository) {}

    void FileWorkspaceService::downloadCurrent(qint64 fileId, std::function<void(ApiResult<QByteArray>)> callback) {
        runSimple(std::move(callback), [fileId](RemoteApiGateway& gateway, auto done) mutable {
            gateway.contentApi().downloadCurrent(fileId, std::move(done));
        });
    }

    void FileWorkspaceService::uploadCurrent(qint64 fileId, QByteArray content, std::function<void(ApiResult<FileVersion>)> callback) {
        runSimple(std::move(callback), [fileId, content = std::move(content)](RemoteApiGateway& gateway, auto done) mutable {
            gateway.contentApi().uploadCurrent(fileId, content, std::move(done));
        });
    }

    void FileWorkspaceService::downloadVersion(qint64 versionId, std::function<void(ApiResult<QByteArray>)> callback) {
        runSimple(std::move(callback), [versionId](RemoteApiGateway& gateway, auto done) mutable {
            gateway.contentApi().downloadVersion(versionId, std::move(done));
        });
    }

    void FileWorkspaceService::acquireLock(qint64 fileId, qint64 lockDurationSec, std::function<void(ApiResult<FileLock>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<FileLock>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* fileRepository = &m_fileRepository;

        m_networkWorker.run([fileId,
                             lockDurationSec,
                             internalContext,
                             uiContext,
                             cb,
                             fileRepository](RemoteApiGateway& gateway) mutable {
            gateway.lockApi().acquire(fileId, lockDurationSec, [fileId, internalContext, uiContext, cb, fileRepository](ApiResult<FileLock> result) mutable {
                ::client::application::postTask(internalContext,
                                      [fileId, uiContext, cb, fileRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              if (auto file = fileRepository->findById(fileId)) {
                                                  file->hasActiveLock = true;
                                                  file->lockedByUserId = result->userId;
                                                  file->lockLeaseUntil = result->leaseUntil;
                                                  fileRepository->upsert(std::move(*file));
                                              }
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void FileWorkspaceService::renewLock(qint64 fileId,
                                         qint64 lockToken,
                                         qint64 lockDurationSec,
                                         std::function<void(ApiResult<void>)> callback) {
        runSimple(std::move(callback), [fileId, lockToken, lockDurationSec](RemoteApiGateway& gateway, auto done) mutable {
            gateway.lockApi().renew(fileId, lockToken, lockDurationSec, std::move(done));
        });
    }

    void FileWorkspaceService::releaseLock(qint64 fileId, qint64 lockToken, std::function<void(ApiResult<void>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* fileRepository = &m_fileRepository;

        m_networkWorker.run([fileId,
                             lockToken,
                             internalContext,
                             uiContext,
                             cb,
                             fileRepository](RemoteApiGateway& gateway) mutable {
            gateway.lockApi().release(fileId, lockToken, [fileId, internalContext, uiContext, cb, fileRepository](ApiResult<void> result) mutable {
                ::client::application::postTask(internalContext,
                                      [fileId, uiContext, cb, fileRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              if (auto file = fileRepository->findById(fileId)) {
                                                  file->hasActiveLock = false;
                                                  file->lockedByUserId.reset();
                                                  file->lockLeaseUntil.reset();
                                                  fileRepository->upsert(std::move(*file));
                                              }
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void FileWorkspaceService::loadVersions(qint64 fileId, std::function<void(ApiResult<std::vector<FileVersion>>)> callback) {
        runSimple(std::move(callback), [fileId](RemoteApiGateway& gateway, auto done) mutable {
            gateway.versionApi().getAll(fileId, std::move(done));
        });
    }

}

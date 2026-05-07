#include "file-catalog-service.h"

#include <algorithm>
#include <expected>
#include <memory>
#include <utility>

#include "domain/models/user-profile.h"
#include "infrastructure/api/file-acl-api.h"

namespace {
using RemoteFile = client::domain::models::RemoteFile;
using FileLock = client::domain::models::FileLock;
using UserFileAcl = client::domain::models::UserFileAcl;
using UserProfile = client::domain::models::UserProfile;
using AclLevel = client::domain::models::AclLevel;
}  // namespace

namespace client::domain::services {

struct FileCatalogService::FileHydrationState {
        std::vector<RemoteFile> files;
        int pending = 0;
        std::shared_ptr<std::function<void(ApiResult<std::vector<RemoteFile>>)>> callback;
};

FileCatalogService::FileCatalogService(application::NetworkWorker& networkWorker, QObject& internalContext,
                                       QObject& uiContext,
                                       infrastructure::repositories::FileRepository& fileRepository) noexcept
    : RemoteServiceBase(networkWorker, internalContext, uiContext), m_fileRepository(fileRepository) {}

void FileCatalogService::refreshFiles(qint64 currentUserId,
                                      std::function<void(ApiResult<std::vector<RemoteFile>>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<std::vector<RemoteFile>>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([currentUserId, internalContext, cb, service](RemoteApiGateway& gateway) mutable {
        gateway.fileApi().getAll(
            [currentUserId, internalContext, cb, service](ApiResult<std::vector<RemoteFile>> result) mutable {
                ::client::application::postTask(
                    internalContext, [currentUserId, cb, service, result = std::move(result)]() mutable {
                        service->hydrateFiles(currentUserId, std::move(result), std::move(cb));
                    });
            });
    });
}

void FileCatalogService::createFile(QString logicalName, quint32 maxVersionCount, qint64 ownerGroupId,
                                    std::function<void(ApiResult<RemoteFile>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<RemoteFile>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* fileRepository = &m_fileRepository;

    m_networkWorker.run([logicalName = std::move(logicalName), maxVersionCount, ownerGroupId, internalContext,
                         uiContext, cb, fileRepository](RemoteApiGateway& gateway) mutable {
        auto* gatewayPtr = &gateway;
        gateway.fileApi().create(
            logicalName, maxVersionCount,
            [ownerGroupId, internalContext, uiContext, cb, fileRepository,
             gatewayPtr](ApiResult<RemoteFile> result) mutable {
                if (!result) {
                    ::client::application::postTask(
                        internalContext, [uiContext, cb, result = std::move(result)]() mutable {
                            ::client::application::postResult(uiContext, cb, std::move(result));
                        });
                    return;
                }

                auto createdFile = *result;
                gatewayPtr->aclApi().setGroupAcl(
                    createdFile.id, ownerGroupId, AclLevel::ReadWrite,
                    [createdFile = std::move(createdFile), internalContext, uiContext, cb,
                     fileRepository](auto aclResult) mutable {
                        ::client::application::postTask(
                            internalContext, [uiContext, cb, fileRepository, createdFile = std::move(createdFile),
                                              aclResult = std::move(aclResult)]() mutable {
                                if (!aclResult) {
                                    ApiResult<RemoteFile> failure = apiFailure(std::move(aclResult.error()));
                                    ::client::application::postResult(uiContext, cb, std::move(failure));
                                    return;
                                }

                                fileRepository->upsert(createdFile);
                                ::client::application::postResult(uiContext, cb, apiSuccess(std::move(createdFile)));
                            });
                    });
            });
    });
}

void FileCatalogService::renameFile(qint64 fileId, QString newLogicalName,
                                    std::function<void(ApiResult<RemoteFile>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<RemoteFile>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* fileRepository = &m_fileRepository;

    m_networkWorker.run([fileId, newLogicalName = std::move(newLogicalName), internalContext, uiContext, cb,
                         fileRepository](RemoteApiGateway& gateway) mutable {
        gateway.fileApi().rename(
            fileId, newLogicalName,
            [internalContext, uiContext, cb, fileRepository](ApiResult<RemoteFile> result) mutable {
                ::client::application::postTask(internalContext,
                                                [uiContext, cb, fileRepository, result = std::move(result)]() mutable {
                                                    if (result) {
                                                        fileRepository->upsert(*result);
                                                    }

                                                    ::client::application::postResult(uiContext, cb, std::move(result));
                                                });
            });
    });
}

void FileCatalogService::deleteFile(qint64 fileId, std::function<void(ApiResult<void>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* fileRepository = &m_fileRepository;

    m_networkWorker.run([fileId, internalContext, uiContext, cb, fileRepository](RemoteApiGateway& gateway) mutable {
        gateway.fileApi().remove(
            fileId, [fileId, internalContext, uiContext, cb, fileRepository](ApiResult<void> result) mutable {
                ::client::application::postTask(
                    internalContext, [fileId, uiContext, cb, fileRepository, result = std::move(result)]() mutable {
                        if (result) {
                            fileRepository->remove(fileId);
                        }

                        ::client::application::postResult(uiContext, cb, std::move(result));
                    });
            });
    });
}

void FileCatalogService::hydrateFiles(
    qint64 currentUserId, ApiResult<std::vector<RemoteFile>> result,
    std::shared_ptr<std::function<void(ApiResult<std::vector<RemoteFile>>)>> callback) {
    if (!result) {
        postUi(std::move(callback), std::move(result));
        return;
    }

    auto state = std::make_shared<FileHydrationState>();
    state->files = std::move(*result);
    state->pending = state->files.size() * 3;
    state->callback = std::move(callback);

    if (state->pending == 0) {
        m_fileRepository.replaceAll({});
        postUi(state->callback, apiSuccess(m_fileRepository.all()));
        return;
    }

    for (const auto& file : state->files) {
        requestCreatorHydration(file.id, file.createdBy, state);
        requestAclHydration(file.id, currentUserId, state);
        requestLockHydration(file.id, state);
    }
}

void FileCatalogService::requestCreatorHydration(qint64 fileId, qint64 userId,
                                                 std::shared_ptr<FileHydrationState> state) {
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([fileId, userId, state, internalContext, service](RemoteApiGateway& gateway) mutable {
        gateway.userApi().getById(
            userId, [fileId, state, internalContext, service](ApiResult<UserProfile> result) mutable {
                ::client::application::postTask(
                    internalContext, [fileId, state, service, result = std::move(result)]() mutable {
                        if (result) {
                            if (auto* file = FileCatalogService::findFile(state->files, fileId)) {
                                file->createdByLogin = result->login;
                            }
                        }

                        service->finishFileHydration(state);
                    });
            });
    });
}

void FileCatalogService::requestAclHydration(qint64 fileId, qint64 currentUserId,
                                             std::shared_ptr<FileHydrationState> state) {
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([fileId, currentUserId, state, internalContext, service](RemoteApiGateway& gateway) mutable {
        gateway.aclApi().getUserAcl(
            fileId, currentUserId, [fileId, state, internalContext, service](ApiResult<UserFileAcl> result) mutable {
                ::client::application::postTask(
                    internalContext, [fileId, state, service, result = std::move(result)]() mutable {
                        if (result) {
                            if (auto* file = FileCatalogService::findFile(state->files, fileId)) {
                                file->aclLevel = result->aclLevel;
                            }
                        }

                        service->finishFileHydration(state);
                    });
            });
    });
}

void FileCatalogService::requestLockHydration(qint64 fileId, std::shared_ptr<FileHydrationState> state) {
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([fileId, state, internalContext, service](RemoteApiGateway& gateway) mutable {
        auto* gatewayPtr = &gateway;
        gateway.lockApi().getActive(
            fileId, [fileId, state, internalContext, service, gatewayPtr](ApiResult<FileLock> result) mutable {
                if (!result) {
                    ::client::application::postTask(internalContext, [fileId, state, service]() mutable {
                        if (auto* file = FileCatalogService::findFile(state->files, fileId)) {
                            file->hasActiveLock = false;
                            file->lockedByUserId.reset();
                            file->lockedByLogin.reset();
                            file->lockLeaseUntil.reset();
                        }

                        service->finishFileHydration(state);
                    });
                    return;
                }

                const auto lock = *result;
                gatewayPtr->userApi().getById(lock.userId, [fileId, lock, state, internalContext,
                                                            service](ApiResult<UserProfile> userResult) mutable {
                    ::client::application::postTask(internalContext, [fileId, lock, state, service,
                                                                      userResult = std::move(userResult)]() mutable {
                        if (auto* file = FileCatalogService::findFile(state->files, fileId)) {
                            file->hasActiveLock = true;
                            file->lockedByUserId = lock.userId;
                            file->lockedByLogin = userResult ? std::optional<QString>{userResult->login} : std::nullopt;
                            file->lockLeaseUntil = lock.leaseUntil;
                        }

                        service->finishFileHydration(state);
                    });
                });
            });
    });
}

void FileCatalogService::finishFileHydration(const std::shared_ptr<FileHydrationState>& state) {
    --state->pending;
    if (state->pending > 0) {
        return;
    }

    std::sort(state->files.begin(), state->files.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.fullLogicalName.localeAwareCompare(rhs.fullLogicalName) < 0;
    });

    m_fileRepository.replaceAll(std::move(state->files));
    postUi(state->callback, apiSuccess(m_fileRepository.all()));
}

RemoteFile* FileCatalogService::findFile(std::vector<RemoteFile>& files, qint64 fileId) {
    auto it = std::find_if(files.begin(), files.end(), [fileId](const auto& file) { return file.id == fileId; });

    return it == files.end() ? nullptr : std::addressof(*it);
}

}  // namespace client::domain::services

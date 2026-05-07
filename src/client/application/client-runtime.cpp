#include "client-runtime.h"

#include "application/network-worker.h"
#include "domain/services/auth-service.h"
#include "domain/services/file-catalog-service.h"
#include "domain/services/file-workspace-service.h"
#include "domain/services/group-sharing-service.h"
#include "domain/services/notification-service.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/session-repository.h"

#include <QMetaObject>
#include <QPointer>

#include <utility>

namespace client::application {

    ClientRuntime::ClientRuntime(QUrl baseUrl, QObject* parent)
        : QObject(parent),
          m_networkWorker(new NetworkWorker(std::move(baseUrl))),
          m_internalContext(new QObject()),
          m_sessionRepository(std::make_unique<infrastructure::repositories::SessionRepository>()),
          m_fileRepository(std::make_unique<infrastructure::repositories::FileRepository>()),
          m_groupRepository(std::make_unique<infrastructure::repositories::GroupRepository>()) {
        m_networkThread.setObjectName(QStringLiteral("client-network-thread"));
        m_internalThread.setObjectName(QStringLiteral("client-internal-thread"));

        m_networkWorker->moveToThread(&m_networkThread);
        m_internalContext->moveToThread(&m_internalThread);

        connect(&m_networkThread, &QThread::finished, m_networkWorker, &QObject::deleteLater);
        connect(&m_internalThread, &QThread::finished, m_internalContext, &QObject::deleteLater);

        m_authService = std::make_unique<domain::services::AuthService>(*m_networkWorker,
                                                                        *m_internalContext,
                                                                        *this,
                                                                        *m_sessionRepository);
        m_fileCatalogService = std::make_unique<domain::services::FileCatalogService>(*m_networkWorker,
                                                                                      *m_internalContext,
                                                                                      *this,
                                                                                      *m_fileRepository);
        m_fileWorkspaceService = std::make_unique<domain::services::FileWorkspaceService>(*m_networkWorker,
                                                                                          *m_internalContext,
                                                                                          *this,
                                                                                          *m_fileRepository);
        m_groupSharingService = std::make_unique<domain::services::GroupSharingService>(*m_networkWorker,
                                                                                        *m_internalContext,
                                                                                        *this,
                                                                                        *m_groupRepository);
        m_notificationService = std::make_unique<domain::services::NotificationService>(*m_networkWorker,
                                                                                        *m_internalContext,
                                                                                        *this);

        m_networkThread.start();
        m_internalThread.start();
    }

    ClientRuntime::~ClientRuntime() {
        if (m_internalContext && m_internalThread.isRunning()) {
            QMetaObject::invokeMethod(m_internalContext,
                                      [this]() {
                                          if (m_notificationService) {
                                              m_notificationService->stop();
                                          }
                                      },
                                      Qt::BlockingQueuedConnection);
        }

        m_internalThread.quit();
        m_internalThread.wait();

        m_authService.reset();
        m_fileCatalogService.reset();
        m_fileWorkspaceService.reset();
        m_groupSharingService.reset();
        m_notificationService.reset();

        if (m_networkWorker && m_networkThread.isRunning()) {
            QPointer<NetworkWorker> worker{m_networkWorker};
            QMetaObject::invokeMethod(m_networkWorker,
                                      [worker]() {
                                          if (worker) {
                                              worker->shutdown();
                                          }
                                      },
                                      Qt::BlockingQueuedConnection);
        }

        m_networkThread.quit();
        m_networkThread.wait();
    }

    template <typename Task>
    void ClientRuntime::postInternal(Task&& task) const {
        QPointer<QObject> guard{m_internalContext};
        QMetaObject::invokeMethod(m_internalContext,
                                  [guard, task = std::forward<Task>(task)]() mutable {
                                      if (!guard) {
                                          return;
                                      }

                                      task();
                                  },
                                  Qt::QueuedConnection);
    }

    void ClientRuntime::authenticate(QUrl baseUrl,
                                     QString login,
                                     QString password,
                                     bool registration) {
        postInternal([this,
                      baseUrl = std::move(baseUrl),
                      login = std::move(login),
                      password = std::move(password),
                      registration]() mutable {
            m_authService->authenticate(std::move(baseUrl),
                                        std::move(login),
                                        std::move(password),
                                        registration,
                                        [this](ApiResult<domain::models::UserSession> result) mutable {
                                            emit authenticationFinished(std::move(result));
                                        });
        });
    }

    void ClientRuntime::setSession(domain::models::UserSession session) {
        postInternal([this, session = std::move(session)]() mutable {
            m_authService->setSession(std::move(session));
        });
    }

    void ClientRuntime::logout() {
        postInternal([this]() {
            m_notificationService->stop();
            m_authService->logout();
        });
    }

    void ClientRuntime::loadFilesWithMeta(qint64 currentUserId) {
        postInternal([this, currentUserId]() {
            m_fileCatalogService->refreshFiles(currentUserId,
                                               [this, currentUserId](ApiResult<std::vector<domain::models::RemoteFile>> result) mutable {
                                                   emit filesLoaded(currentUserId, std::move(result));
                                               });
        });
    }

    void ClientRuntime::createFile(QString logicalName, quint32 maxVersionCount) {
        postInternal([this, logicalName = std::move(logicalName), maxVersionCount]() mutable {
            m_fileCatalogService->createFile(std::move(logicalName),
                                             maxVersionCount,
                                             [this](ApiResult<domain::models::RemoteFile> result) mutable {
                                                 emit fileCreated(std::move(result));
                                             });
        });
    }

    void ClientRuntime::renameFile(qint64 fileId, QString newLogicalName) {
        postInternal([this, fileId, newLogicalName = std::move(newLogicalName)]() mutable {
            m_fileCatalogService->renameFile(fileId,
                                             std::move(newLogicalName),
                                             [this, fileId](ApiResult<domain::models::RemoteFile> result) mutable {
                                                 emit fileRenamed(fileId, std::move(result));
                                             });
        });
    }

    void ClientRuntime::deleteFile(qint64 fileId) {
        postInternal([this, fileId]() {
            m_fileCatalogService->deleteFile(fileId,
                                             [this, fileId](ApiResult<void> result) mutable {
                                                 emit fileDeleted(fileId, std::move(result));
                                             });
        });
    }

    void ClientRuntime::downloadCurrent(qint64 fileId) {
        postInternal([this, fileId]() {
            m_fileWorkspaceService->downloadCurrent(fileId,
                                                    [this, fileId](ApiResult<QByteArray> result) mutable {
                                                        emit currentDownloaded(fileId, std::move(result));
                                                    });
        });
    }

    void ClientRuntime::uploadCurrent(qint64 fileId, QByteArray content) {
        postInternal([this, fileId, content = std::move(content)]() mutable {
            m_fileWorkspaceService->uploadCurrent(fileId,
                                                  std::move(content),
                                                  [this, fileId](ApiResult<domain::models::FileVersion> result) mutable {
                                                      emit currentUploaded(fileId, std::move(result));
                                                  });
        });
    }

    void ClientRuntime::downloadVersion(qint64 versionId) {
        postInternal([this, versionId]() {
            m_fileWorkspaceService->downloadVersion(versionId,
                                                    [this, versionId](ApiResult<QByteArray> result) mutable {
                                                        emit versionDownloaded(versionId, std::move(result));
                                                    });
        });
    }

    void ClientRuntime::acquireLock(qint64 fileId, qint64 lockDurationSec) {
        postInternal([this, fileId, lockDurationSec]() {
            m_fileWorkspaceService->acquireLock(fileId,
                                                lockDurationSec,
                                                [this, fileId](ApiResult<domain::models::FileLock> result) mutable {
                                                    emit lockAcquired(fileId, std::move(result));
                                                });
        });
    }

    void ClientRuntime::renewLock(qint64 fileId, qint64 lockToken, qint64 lockDurationSec) {
        postInternal([this, fileId, lockToken, lockDurationSec]() {
            m_fileWorkspaceService->renewLock(fileId,
                                              lockToken,
                                              lockDurationSec,
                                              [this, fileId, lockToken](ApiResult<void> result) mutable {
                                                  emit lockRenewed(fileId, lockToken, std::move(result));
                                              });
        });
    }

    void ClientRuntime::releaseLock(qint64 fileId, qint64 lockToken) {
        postInternal([this, fileId, lockToken]() {
            m_fileWorkspaceService->releaseLock(fileId,
                                                lockToken,
                                                [this, fileId, lockToken](ApiResult<void> result) mutable {
                                                    emit lockReleased(fileId, lockToken, std::move(result));
                                                });
        });
    }

    void ClientRuntime::loadVersions(qint64 fileId) {
        postInternal([this, fileId]() {
            m_fileWorkspaceService->loadVersions(fileId,
                                                 [this, fileId](ApiResult<std::vector<domain::models::FileVersion>> result) mutable {
                                                     emit versionsLoaded(fileId, std::move(result));
                                                 });
        });
    }

    void ClientRuntime::loadCurrentUserGroups(qint64 currentUserId) {
        postInternal([this, currentUserId]() {
            m_groupSharingService->loadCurrentUserGroups(currentUserId,
                                                         [this, currentUserId](ApiResult<std::vector<domain::models::Group>> result) mutable {
                                                             emit currentUserGroupsLoaded(currentUserId, std::move(result));
                                                         });
        });
    }

    void ClientRuntime::loadGroupUsers(qint64 groupId) {
        postInternal([this, groupId]() {
            m_groupSharingService->loadGroupUsers(groupId,
                                                  [this, groupId](ApiResult<std::vector<domain::models::UserProfile>> result) mutable {
                                                      emit groupUsersLoaded(groupId, std::move(result));
                                                  });
        });
    }

    void ClientRuntime::addUserToGroup(QString login, qint64 groupId) {
        postInternal([this, login = std::move(login), groupId]() mutable {
            const auto requestedLogin = login;
            m_groupSharingService->addUserToGroup(std::move(login),
                                                  groupId,
                                                  [this, requestedLogin, groupId](ApiResult<void> result) mutable {
                                                      emit userAddedToGroup(requestedLogin, groupId, std::move(result));
                                                  });
        });
    }

    void ClientRuntime::removeUserFromGroup(qint64 userId, qint64 groupId) {
        postInternal([this, userId, groupId]() {
            m_groupSharingService->removeUserFromGroup(userId,
                                                       groupId,
                                                       [this, userId, groupId](ApiResult<void> result) mutable {
                                                           emit userRemovedFromGroup(userId, groupId, std::move(result));
                                                       });
        });
    }

    void ClientRuntime::createGroupForCurrentUser(QString name, qint64 currentUserId) {
        postInternal([this, name = std::move(name), currentUserId]() mutable {
            m_groupSharingService->createGroupForCurrentUser(std::move(name),
                                                             currentUserId,
                                                             [this](ApiResult<domain::models::Group> result) mutable {
                                                                 emit groupCreated(std::move(result));
                                                             });
        });
    }

    void ClientRuntime::startNotifications() {
        postInternal([this]() {
            m_notificationService->start([this](ApiResult<domain::models::NotificationEvent> result) mutable {
                emit notificationReceived(std::move(result));
            });
        });
    }

    void ClientRuntime::stopNotifications() {
        if (!m_internalContext) {
            return;
        }

        postInternal([this]() {
            m_notificationService->stop();
        });
    }

}

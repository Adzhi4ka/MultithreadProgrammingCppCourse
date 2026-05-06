#pragma once

#include "domain/models/file-lock.h"
#include "domain/models/file-version.h"
#include "domain/models/group.h"
#include "domain/models/notification-event.h"
#include "domain/models/remote-file.h"
#include "domain/models/user-session.h"
#include "infrastructure/api/api-result.h"

#include <QByteArray>
#include <QObject>
#include <QThread>
#include <QUrl>

#include <memory>
#include <vector>

namespace client::application {

    class NetworkWorker;
}

namespace client::domain::services {
    class AuthService;
    class FileCatalogService;
    class FileWorkspaceService;
    class GroupSharingService;
    class NotificationService;
}

namespace client::infrastructure::repositories {
    class FileRepository;
    class GroupRepository;
    class SessionRepository;
}

namespace client::application {

    class ClientRuntime : public QObject {

            Q_OBJECT

            QThread m_networkThread;
            QThread m_internalThread;

            NetworkWorker* m_networkWorker = nullptr;
            QObject* m_internalContext = nullptr;

            std::unique_ptr<infrastructure::repositories::SessionRepository> m_sessionRepository;
            std::unique_ptr<infrastructure::repositories::FileRepository> m_fileRepository;
            std::unique_ptr<infrastructure::repositories::GroupRepository> m_groupRepository;

            std::unique_ptr<domain::services::AuthService> m_authService;
            std::unique_ptr<domain::services::FileCatalogService> m_fileCatalogService;
            std::unique_ptr<domain::services::FileWorkspaceService> m_fileWorkspaceService;
            std::unique_ptr<domain::services::GroupSharingService> m_groupSharingService;
            std::unique_ptr<domain::services::NotificationService> m_notificationService;

        public:
            explicit ClientRuntime(QUrl baseUrl, QObject* parent = nullptr);
            ~ClientRuntime() override;

            ClientRuntime(const ClientRuntime&) = delete;
            ClientRuntime& operator=(const ClientRuntime&) = delete;

        public slots:
            void authenticate(QUrl baseUrl,
                              QString login,
                              QString password,
                              bool registration);
            void setSession(domain::models::UserSession session);
            void logout();

            void loadFilesWithMeta(qint64 currentUserId);
            void createFile(QString logicalName, quint32 maxVersionCount);
            void renameFile(qint64 fileId, QString newLogicalName);
            void deleteFile(qint64 fileId);

            void downloadCurrent(qint64 fileId);
            void uploadCurrent(qint64 fileId, QByteArray content);
            void downloadVersion(qint64 versionId);

            void acquireLock(qint64 fileId, qint64 lockDurationSec);
            void renewLock(qint64 fileId, qint64 lockToken, qint64 lockDurationSec);
            void releaseLock(qint64 fileId, qint64 lockToken);

            void loadVersions(qint64 fileId);

            void loadCurrentUserGroups(qint64 currentUserId);
            void loadGroupUsers(qint64 groupId);
            void addUserToGroup(qint64 userId, qint64 groupId);
            void removeUserFromGroup(qint64 userId, qint64 groupId);
            void createGroupForCurrentUser(QString name, qint64 currentUserId);

            void startNotifications();
            void stopNotifications();

        signals:

            void authenticationFinished(ApiResult<domain::models::UserSession> result);

            void filesLoaded(qint64 currentUserId, ApiResult<std::vector<domain::models::RemoteFile>> result);
            void fileCreated(ApiResult<domain::models::RemoteFile> result);
            void fileRenamed(qint64 fileId, ApiResult<domain::models::RemoteFile> result);
            void fileDeleted(qint64 fileId, ApiResult<void> result);

            void currentDownloaded(qint64 fileId, ApiResult<QByteArray> result);
            void currentUploaded(qint64 fileId, ApiResult<domain::models::FileVersion> result);
            void versionDownloaded(qint64 versionId, ApiResult<QByteArray> result);

            void lockAcquired(qint64 fileId, ApiResult<domain::models::FileLock> result);
            void lockRenewed(qint64 fileId, qint64 lockToken, ApiResult<void> result);
            void lockReleased(qint64 fileId, qint64 lockToken, ApiResult<void> result);

            void versionsLoaded(qint64 fileId, ApiResult<std::vector<domain::models::FileVersion>> result);

            void currentUserGroupsLoaded(qint64 currentUserId, ApiResult<std::vector<domain::models::Group>> result);
            void groupUsersLoaded(qint64 groupId, ApiResult<std::vector<qint64>> result);
            void userAddedToGroup(qint64 userId, qint64 groupId, ApiResult<void> result);
            void userRemovedFromGroup(qint64 userId, qint64 groupId, ApiResult<void> result);
            void groupCreated(ApiResult<domain::models::Group> result);

            void notificationReceived(ApiResult<domain::models::NotificationEvent> result);

        private:

            template <typename Task>
            void postInternal(Task&& task) const;

    };

}

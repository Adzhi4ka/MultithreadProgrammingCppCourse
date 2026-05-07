#pragma once

#include "application/client-runtime.h"
#include "domain/models/group.h"
#include "domain/models/user-session.h"
#include "domain/models/user-profile.h"

#include <QDialog>
#include <optional>
#include <vector>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

namespace client::presentation {

    class GroupManagementDialog : public QDialog {

            Q_OBJECT

            application::ClientRuntime& m_runtime;
            domain::models::UserSession m_session;

            QListWidget* m_groupList = nullptr;
            QListWidget* m_userList = nullptr;
            QLineEdit* m_targetLoginEdit = nullptr;
            QLabel* m_statusLabel = nullptr;
            QPushButton* m_refreshButton = nullptr;
            QPushButton* m_createGroupButton = nullptr;
            QPushButton* m_addUserButton = nullptr;
            QPushButton* m_removeUserButton = nullptr;

        public:
            explicit GroupManagementDialog(application::ClientRuntime& runtime,
                                           domain::models::UserSession session,
                                           QWidget* parent = nullptr);

        private slots:
            void refreshGroups();
            void refreshUsers();
            void addUserToSelectedGroup();
            void removeSelectedUserFromGroup();
            void createGroup();

            void handleCurrentUserGroupsLoaded(qint64 currentUserId, ApiResult<std::vector<domain::models::Group>> result);
            void handleGroupUsersLoaded(qint64 groupId, ApiResult<std::vector<domain::models::UserProfile>> result);
            void handleUserAddedToGroup(QString login, qint64 groupId, ApiResult<void> result);
            void handleUserRemovedFromGroup(qint64 userId, qint64 groupId, ApiResult<void> result);
            void handleGroupCreated(ApiResult<domain::models::Group> result);

        private:
            void buildUi();
            void connectRuntimeSignals();
            std::optional<qint64> selectedGroupId() const;
            std::optional<qint64> selectedUserId() const;
            void setBusy(bool busy);
            void showApiError(const QString& title, const ApiError& error);

    };

}

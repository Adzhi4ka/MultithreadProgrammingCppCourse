#include "group-management-dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <optional>
#include <utility>

namespace client::presentation {

    namespace {
        constexpr int GroupIdRole = Qt::UserRole + 1;
        constexpr int UserIdRole = Qt::UserRole + 2;
    }

    GroupManagementDialog::GroupManagementDialog(application::ClientRuntime& runtime,
                                                 domain::models::UserSession session,
                                                 QWidget* parent)
        : QDialog(parent),
          m_runtime(runtime),
          m_session(std::move(session)) {
        buildUi();
        connectRuntimeSignals();
        refreshGroups();
    }

    void GroupManagementDialog::buildUi() {
        setWindowTitle(QStringLiteral("Groups and users"));
        resize(760, 480);

        m_groupList = new QListWidget(this);
        m_userList = new QListWidget(this);
        m_targetLoginEdit = new QLineEdit(this);
        m_targetLoginEdit->setPlaceholderText(QStringLiteral("Target user login"));
        m_statusLabel = new QLabel(this);
        m_statusLabel->setWordWrap(true);

        m_refreshButton = new QPushButton(QStringLiteral("Refresh"), this);
        m_createGroupButton = new QPushButton(QStringLiteral("Create my group"), this);
        m_addUserButton = new QPushButton(QStringLiteral("Add user to group"), this);
        m_removeUserButton = new QPushButton(QStringLiteral("Remove selected user"), this);

        auto* leftPanel = new QWidget(this);
        auto* leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->addWidget(new QLabel(QStringLiteral("Your groups"), this));
        leftLayout->addWidget(m_groupList, 1);
        leftLayout->addWidget(m_createGroupButton);

        auto* rightPanel = new QWidget(this);
        auto* rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->addWidget(new QLabel(QStringLiteral("Users in selected group"), this));
        rightLayout->addWidget(m_userList, 1);

        auto* userForm = new QFormLayout;
        userForm->addRow(QStringLiteral("Login"), m_targetLoginEdit);
        rightLayout->addLayout(userForm);
        rightLayout->addWidget(m_addUserButton);
        rightLayout->addWidget(m_removeUserButton);

        auto* splitter = new QSplitter(this);
        splitter->addWidget(leftPanel);
        splitter->addWidget(rightPanel);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
        auto* topButtons = new QHBoxLayout;
        topButtons->addWidget(m_refreshButton);
        topButtons->addStretch();

        auto* root = new QVBoxLayout(this);
        root->addLayout(topButtons);
        root->addWidget(splitter, 1);
        root->addWidget(m_statusLabel);
        root->addWidget(buttons);

        connect(m_refreshButton, &QPushButton::clicked, this, &GroupManagementDialog::refreshGroups);
        connect(m_createGroupButton, &QPushButton::clicked, this, &GroupManagementDialog::createGroup);
        connect(m_addUserButton, &QPushButton::clicked, this, &GroupManagementDialog::addUserToSelectedGroup);
        connect(m_removeUserButton, &QPushButton::clicked, this, &GroupManagementDialog::removeSelectedUserFromGroup);
        connect(m_groupList, &QListWidget::currentRowChanged, this, &GroupManagementDialog::refreshUsers);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    void GroupManagementDialog::connectRuntimeSignals() {
        connect(&m_runtime,
                &application::ClientRuntime::currentUserGroupsLoaded,
                this,
                &GroupManagementDialog::handleCurrentUserGroupsLoaded);
        connect(&m_runtime, &application::ClientRuntime::groupUsersLoaded, this, &GroupManagementDialog::handleGroupUsersLoaded);
        connect(&m_runtime, &application::ClientRuntime::userAddedToGroup, this, &GroupManagementDialog::handleUserAddedToGroup);
        connect(&m_runtime, &application::ClientRuntime::userRemovedFromGroup, this, &GroupManagementDialog::handleUserRemovedFromGroup);
        connect(&m_runtime, &application::ClientRuntime::groupCreated, this, &GroupManagementDialog::handleGroupCreated);
    }

    void GroupManagementDialog::refreshGroups() {
        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Loading groups..."));
        m_runtime.loadCurrentUserGroups(m_session.userId);
    }

    void GroupManagementDialog::refreshUsers() {
        const auto groupId = selectedGroupId();
        if (!groupId) {
            m_userList->clear();
            return;
        }

        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Loading users..."));
        m_runtime.loadGroupUsers(*groupId);
    }

    void GroupManagementDialog::addUserToSelectedGroup() {
        const auto groupId = selectedGroupId();
        if (!groupId) {
            return;
        }

        const auto login = m_targetLoginEdit->text().trimmed();
        if (login.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Invalid login"), QStringLiteral("Enter user login"));
            return;
        }

        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Adding user..."));
        m_runtime.addUserToGroup(login, *groupId);
    }

    void GroupManagementDialog::removeSelectedUserFromGroup() {
        const auto groupId = selectedGroupId();
        const auto userId = selectedUserId();
        if (!groupId || !userId) {
            return;
        }

        if (*userId == m_session.userId) {
            if (QMessageBox::question(this,
                                      QStringLiteral("Remove yourself"),
                                      QStringLiteral("Remove current user from this group?")) != QMessageBox::Yes) {
                return;
            }
        }

        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Removing user..."));
        m_runtime.removeUserFromGroup(*userId, *groupId);
    }

    void GroupManagementDialog::createGroup() {
        bool ok = false;
        const auto name = QInputDialog::getText(this,
                                                QStringLiteral("Create group"),
                                                QStringLiteral("Group name"),
                                                QLineEdit::Normal,
                                                QString{},
                                                &ok).trimmed();
        if (!ok || name.isEmpty()) {
            return;
        }

        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Creating group..."));
        m_runtime.createGroupForCurrentUser(name, m_session.userId);
    }

    void GroupManagementDialog::handleCurrentUserGroupsLoaded(qint64 currentUserId,
                                                              ApiResult<std::vector<domain::models::Group>> result) {
        if (currentUserId != m_session.userId) {
            return;
        }

        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to load groups"), result.error());
            m_statusLabel->clear();
            return;
        }

        m_groupList->clear();
        for (const auto& group : *result) {
            auto* item = new QListWidgetItem(QStringLiteral("%1  (id=%2)").arg(group.name).arg(group.id), m_groupList);
            item->setData(GroupIdRole, group.id);
        }

        m_statusLabel->setText(QStringLiteral("Loaded %1 groups").arg(result->size()));
        if (m_groupList->count() > 0) {
            m_groupList->setCurrentRow(0);
        } else {
            m_userList->clear();
        }
    }

    void GroupManagementDialog::handleGroupUsersLoaded(qint64 groupId, ApiResult<std::vector<domain::models::UserProfile>> result) {
        const auto selected = selectedGroupId();
        if (!selected || *selected != groupId) {
            return;
        }

        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to load group users"), result.error());
            m_statusLabel->clear();
            return;
        }

        m_userList->clear();
        for (const auto& user : *result) {
            auto* item = new QListWidgetItem(user.login, m_userList);
            item->setData(UserIdRole, user.userId);
        }

        m_statusLabel->setText(QStringLiteral("Loaded %1 users").arg(result->size()));
    }

    void GroupManagementDialog::handleUserAddedToGroup(QString, qint64 groupId, ApiResult<void> result) {
        const auto selected = selectedGroupId();
        if (!selected || *selected != groupId) {
            return;
        }

        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to add user"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshUsers();
    }

    void GroupManagementDialog::handleUserRemovedFromGroup(qint64, qint64 groupId, ApiResult<void> result) {
        const auto selected = selectedGroupId();
        if (!selected || *selected != groupId) {
            return;
        }

        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to remove user"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshUsers();
    }

    void GroupManagementDialog::handleGroupCreated(ApiResult<domain::models::Group> result) {
        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to create group"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshGroups();
    }

    std::optional<qint64> GroupManagementDialog::selectedGroupId() const {
        const auto* item = m_groupList->currentItem();
        if (!item) {
            return std::nullopt;
        }

        const auto value = item->data(GroupIdRole);
        return value.isValid() ? std::optional<qint64>{value.toLongLong()} : std::nullopt;
    }

    std::optional<qint64> GroupManagementDialog::selectedUserId() const {
        const auto* item = m_userList->currentItem();
        if (!item) {
            return std::nullopt;
        }

        const auto value = item->data(UserIdRole);
        return value.isValid() ? std::optional<qint64>{value.toLongLong()} : std::nullopt;
    }

    void GroupManagementDialog::setBusy(bool busy) {
        m_refreshButton->setEnabled(!busy);
        m_createGroupButton->setEnabled(!busy);
        m_addUserButton->setEnabled(!busy);
        m_removeUserButton->setEnabled(!busy);
        m_groupList->setEnabled(!busy);
        m_userList->setEnabled(!busy);
        m_targetLoginEdit->setEnabled(!busy);
    }

    void GroupManagementDialog::showApiError(const QString& title, const ApiError& error) {
        const auto message = QStringLiteral("%1%2")
            .arg(error.httpStatus > 0 ? QStringLiteral("HTTP %1: ").arg(error.httpStatus) : QStringLiteral("Network error: "))
            .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);
        QMessageBox::warning(this, title, message);
    }

}

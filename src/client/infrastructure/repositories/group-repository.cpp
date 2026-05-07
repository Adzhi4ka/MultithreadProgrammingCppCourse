#include "group-repository.h"

#include <algorithm>
#include <utility>

namespace {
    using Group = client::domain::models::Group;
    using UserProfile = client::domain::models::UserProfile;
}

namespace client::infrastructure::repositories {

    void GroupRepository::replaceGroups(std::vector<Group> groups) {
        m_groupsById.clear();
        for (auto& group : groups) {
            m_groupsById.insert(group.id, std::move(group));
        }
    }

    void GroupRepository::upsertGroup(Group group) {
        m_groupsById.insert(group.id, std::move(group));
    }

    void GroupRepository::upsertUser(UserProfile user) {
        m_usersById.insert(user.userId, std::move(user));
    }

    void GroupRepository::replaceGroupUsers(qint64 groupId, std::vector<UserProfile> users) {
        for (const auto& user : users) {
            if (user.isValid()) {
                m_usersById.insert(user.userId, user);
            }
        }

        std::sort(users.begin(), users.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.userId < rhs.userId;
        });

        users.erase(std::unique(users.begin(), users.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.userId == rhs.userId;
        }), users.end());

        std::sort(users.begin(), users.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.login.localeAwareCompare(rhs.login) < 0;
        });

        m_usersByGroupId.insert(groupId, std::move(users));
    }

    std::optional<Group> GroupRepository::findGroup(qint64 groupId) const {
        const auto it = m_groupsById.constFind(groupId);
        if (it == m_groupsById.constEnd()) {
            return std::nullopt;
        }

        return *it;
    }

    std::optional<UserProfile> GroupRepository::findUser(qint64 userId) const {
        const auto it = m_usersById.constFind(userId);
        if (it == m_usersById.constEnd()) {
            return std::nullopt;
        }

        return *it;
    }

    std::vector<Group> GroupRepository::groups() const {
        std::vector<Group> result;
        result.reserve(m_groupsById.size());

        for (auto it = m_groupsById.constBegin(); it != m_groupsById.constEnd(); ++it) {
            result.emplace_back(*it);
        }

        std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.name.localeAwareCompare(rhs.name) < 0;
        });

        return result;
    }

    std::vector<UserProfile> GroupRepository::groupUsers(qint64 groupId) const {
        const auto it = m_usersByGroupId.constFind(groupId);
        if (it == m_usersByGroupId.constEnd()) {
            return {};
        }

        return *it;
    }

}

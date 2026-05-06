#include "group-repository.h"

#include <algorithm>
#include <utility>

namespace {
    using Group = client::domain::models::Group;
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

    void GroupRepository::replaceGroupUsers(qint64 groupId, std::vector<qint64> users) {
        std::sort(users.begin(), users.end());
        users.erase(std::unique(users.begin(), users.end()), users.end());
        m_usersByGroupId.insert(groupId, std::move(users));
    }

    std::optional<Group> GroupRepository::findGroup(qint64 groupId) const {
        const auto it = m_groupsById.constFind(groupId);
        if (it == m_groupsById.constEnd()) {
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

    std::vector<qint64> GroupRepository::groupUsers(qint64 groupId) const {
        const auto it = m_usersByGroupId.constFind(groupId);
        if (it == m_usersByGroupId.constEnd()) {
            return {};
        }

        return *it;
    }

}

#pragma once

#include <QHash>
#include <optional>
#include <vector>

#include "domain/models/group.h"
#include "domain/models/user-profile.h"

namespace client::infrastructure::repositories {

class GroupRepository {

        using Group = domain::models::Group;
        using UserProfile = domain::models::UserProfile;

        QHash<qint64, Group> m_groupsById;
        QHash<qint64, UserProfile> m_usersById;
        QHash<qint64, std::vector<UserProfile>> m_usersByGroupId;

    public:

        void replaceGroups(std::vector<Group> groups);
        void upsertGroup(Group group);
        void upsertUser(UserProfile user);
        void replaceGroupUsers(qint64 groupId, std::vector<UserProfile> users);

        std::optional<Group> findGroup(qint64 groupId) const;
        std::optional<UserProfile> findUser(qint64 userId) const;
        std::vector<Group> groups() const;
        std::vector<UserProfile> groupUsers(qint64 groupId) const;
};

}  // namespace client::infrastructure::repositories

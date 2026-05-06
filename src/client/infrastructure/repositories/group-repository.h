#pragma once

#include "domain/models/group.h"

#include <QHash>

#include <optional>
#include <vector>

namespace client::infrastructure::repositories {

    class GroupRepository {

            using Group = domain::models::Group;

            QHash<qint64, Group> m_groupsById;
            QHash<qint64, std::vector<qint64>> m_usersByGroupId;

        public:

            void replaceGroups(std::vector<Group> groups);
            void upsertGroup(Group group);
            void replaceGroupUsers(qint64 groupId, std::vector<qint64> users);

            std::optional<Group> findGroup(qint64 groupId) const;
            std::vector<Group> groups() const;
            std::vector<qint64> groupUsers(qint64 groupId) const;

    };

}

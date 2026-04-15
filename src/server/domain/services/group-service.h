#pragma once

#include "domain/models/group.h"
#include "domain/models/user-group.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/repositories/group-repository.h"
#include "infrastructure/database/repositories/user-group-repository.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace domain::models;

    class GroupService {

            sqlite::SqliteDatabase& m_database;

            repositories::GroupRepository& m_groupRepo;

            repositories::UserGroupRepository& m_userGroupRepo;

        public:

            int64_t createGroup(const std::string& groupName) {
                auto wouw = m_database.createWriteUnitOfWork();
                
                int64_t newId = (int64_t)std::hash<std::string>{}(groupName);
                m_groupRepo.create(wouw, Group{.id = newId, .name = groupName});
                wouw.commit();

                return newId;
            }

            void addUserToGroup(int64_t userId, int64_t groupId) {
                auto wouw = m_database.createWriteUnitOfWork();

                m_userGroupRepo.addUserToGroup(wouw, UserGroup{.userId = userId, .groupId = groupId});

                wouw.commit();
            }

            void removeUserFromGroup(int64_t userId, int64_t groupId) {
                auto wouw = m_database.createWriteUnitOfWork();

                m_userGroupRepo.removeUserFromGroup(wouw, UserGroup{.userId = userId, .groupId = groupId});

                wouw.commit();
            }

            std::vector<int64_t> getUserGroups(int64_t userId) {
                auto rouw = m_database.createReadUnitOfWork();

                return m_userGroupRepo.getGroupIdsOfUser(rouw, userId);
            }

    };

}
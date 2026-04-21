#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-group-repository.h"

#include <cstdint>
#include <vector>

namespace domain::services {

    class GroupService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using GroupRepository = infrastructure::repositories::GroupRepository;
            using UserGroupRepository = infrastructure::repositories::UserGroupRepository;

            SqliteDatabase& m_database;
            GroupRepository& m_groupRepo;
            UserGroupRepository& m_userGroupRepo;

        public:

            GroupService(SqliteDatabase& database,
                         GroupRepository& groupRepo,
                         UserGroupRepository& userGroupRepo) noexcept;

            ServiceResult<int64_t> createGroup(const std::string& groupName);

            ServiceResult<void> addUserToGroup(int64_t userId, int64_t groupId);

            ServiceResult<void> removeUserFromGroup(int64_t userId, int64_t groupId);

            ServiceResult<std::vector<int64_t>> getUserGroups(int64_t userId);

    };

}
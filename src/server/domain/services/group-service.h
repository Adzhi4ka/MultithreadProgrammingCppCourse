#pragma once

#include "domain/models/group.h"
#include "domain/models/user-group.h"
#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/id-generator/id-generator.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-group-repository.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace infrastructure::repositories;
    using namespace domain::models;

    class GroupService {

            sqlite::SqliteDatabase& m_database;
            GroupRepository& m_groupRepo;
            UserGroupRepository& m_userGroupRepo;

        public:

            GroupService(sqlite::SqliteDatabase& database,
                         GroupRepository& groupRepo,
                         UserGroupRepository& userGroupRepo) noexcept;

            ServiceResult<int64_t> createGroup(const std::string& groupName);

            ServiceResult<void> addUserToGroup(int64_t userId, int64_t groupId);

            ServiceResult<void> removeUserFromGroup(int64_t userId, int64_t groupId);

            ServiceResult<std::vector<int64_t>> getUserGroups(int64_t userId);

    };

}
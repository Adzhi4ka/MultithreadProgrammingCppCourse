#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/user.h"
#include "domain/models/group.h"
#include "domain/models/user-group.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class UserGroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserGroupRepository(SqliteDatabase& db) : m_db(db) {}

            void addUserToGroup(WriteUnitOfWork& wuov, UserGroup userGroup);
            void removeUserFromGroup(WriteUnitOfWork& wuov, UserGroup userGroup);

            std::vector<int64_t> getGroupIdsOfUser(UnitOfWork& uow, int64_t userId);
            std::vector<int64_t> getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId);

    };

}
#pragma once

#include "infrastructure/database/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/user-group.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class UserGroupRepository {

            SqliteDatabase& m_db;

        public:

            explicit UserGroupRepository(SqliteDatabase& db) : m_db(db) {}

            PersistenceResult<void> addUserToGroup(WriteUnitOfWork& wuov, UserGroup userGroup);
            PersistenceResult<void> removeUserFromGroup(WriteUnitOfWork& wuov, UserGroup userGroup);

            PersistenceResult<std::vector<int64_t>> getGroupIdsOfUser(UnitOfWork& uow, int64_t userId);
            PersistenceResult<std::vector<int64_t>> getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId);

    };

}
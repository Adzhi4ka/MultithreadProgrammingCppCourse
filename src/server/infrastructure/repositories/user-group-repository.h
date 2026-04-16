#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/user-group.h"

#include <cstdint>

namespace infrastructure::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class UserGroupRepository {

        public:

            PersistenceResult<void> addUserToGroup(WriteUnitOfWork& wuov, UserGroup userGroup);
            PersistenceResult<void> removeUserFromGroup(WriteUnitOfWork& wuov, UserGroup userGroup);

            PersistenceResult<std::vector<int64_t>> getGroupIdsOfUser(UnitOfWork& uow, int64_t userId);
            PersistenceResult<std::vector<int64_t>> getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId);

    };

}
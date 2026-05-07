#pragma once

#include <cstdint>

#include "domain/models/user-group.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/persistence-result.h"

namespace infrastructure::repositories {

class UserGroupRepository {

        using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
        using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
        using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
        using UserGroup = domain::models::UserGroup;

    public:

        PersistenceResult<void> addUserToGroup(WriteUnitOfWork& wuow, UserGroup userGroup);
        PersistenceResult<void> removeUserFromGroup(WriteUnitOfWork& wuow, UserGroup userGroup);

        PersistenceResult<std::vector<int64_t>> getGroupIdsOfUser(UnitOfWork& uow, int64_t userId);
        PersistenceResult<std::vector<int64_t>> getUserIdsOfGroup(UnitOfWork& uow, int64_t groupId);
};

}  // namespace infrastructure::repositories
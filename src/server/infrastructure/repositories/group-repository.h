#pragma once

#include <cstdint>

#include "domain/models/group.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/persistence-result.h"

namespace infrastructure::repositories {

class GroupRepository {

        using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
        using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
        using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
        using Group = domain::models::Group;

    public:

        PersistenceResult<void> create(WriteUnitOfWork& wuow, Group group);
        PersistenceResult<void> remove(WriteUnitOfWork& wuow, int64_t id);

        PersistenceResult<Group> getById(UnitOfWork& uow, int64_t id);
        PersistenceResult<Group> getByName(UnitOfWork& uow, const std::string& name);
        PersistenceResult<std::vector<Group>> getAll(UnitOfWork& uow);
};

}  // namespace infrastructure::repositories
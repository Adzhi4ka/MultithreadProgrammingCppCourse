#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/group.h"

#include <cstdint>

namespace infrastructure::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class GroupRepository {

        public:

            PersistenceResult<void> create(WriteUnitOfWork& wuov, Group group);
            PersistenceResult<void> remove(WriteUnitOfWork& wuov, int64_t id);

            PersistenceResult<Group> getById(UnitOfWork& uow, int64_t id);
            PersistenceResult<Group> getByName(UnitOfWork& uow, const std::string& name);
            PersistenceResult<std::vector<Group>> getAll(UnitOfWork& uow);

    };

}
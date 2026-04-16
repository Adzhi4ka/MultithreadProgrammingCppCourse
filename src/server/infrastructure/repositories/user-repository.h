#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/user.h"

#include <cstdint>

namespace infrastructure::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class UserRepository {

        public:

            PersistenceResult<void> create(WriteUnitOfWork& wuov, User user);
            PersistenceResult<void> update(WriteUnitOfWork& wuov, User user);
            PersistenceResult<void> remove(WriteUnitOfWork& wuov, int64_t id);

            PersistenceResult<User> getById(UnitOfWork& uow, int64_t id);
            PersistenceResult<User> getByLogin(UnitOfWork& uow, const std::string& login);
            PersistenceResult<std::vector<User>> getAll(UnitOfWork& uow);
    };

}
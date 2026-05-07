#pragma once

#include <cstdint>

#include "domain/models/user.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/persistence-result.h"

namespace infrastructure::repositories {

class UserRepository {

        using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
        using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
        using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
        using User = domain::models::User;

    public:

        PersistenceResult<void> create(WriteUnitOfWork& wuow, User user);
        PersistenceResult<void> update(WriteUnitOfWork& wuow, User user);
        PersistenceResult<void> remove(WriteUnitOfWork& wuow, int64_t id);

        PersistenceResult<User> getById(UnitOfWork& uow, int64_t id);
        PersistenceResult<User> getByLogin(UnitOfWork& uow, const std::string& login);
        PersistenceResult<std::vector<User>> getAll(UnitOfWork& uow);
};

}  // namespace infrastructure::repositories
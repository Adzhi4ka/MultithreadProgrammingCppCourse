#pragma once

#include <cstdint>

#include "domain/models/file.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/persistence-result.h"

namespace infrastructure::repositories {

class FileRepository {

        using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
        using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
        using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
        using File = domain::models::File;

    public:

        PersistenceResult<void> create(WriteUnitOfWork& wuow, File file);
        PersistenceResult<void> update(WriteUnitOfWork& wuow, File file);
        PersistenceResult<void> remove(WriteUnitOfWork& wuow, int64_t id);

        PersistenceResult<File> getById(UnitOfWork& uow, int64_t id);
        PersistenceResult<File> getByName(UnitOfWork& uow, const std::string& name);
        PersistenceResult<std::vector<File>> getAll(UnitOfWork& uow);
};

}  // namespace infrastructure::repositories
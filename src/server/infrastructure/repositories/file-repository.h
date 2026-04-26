#pragma once

#include "infrastructure/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file.h"

#include <cstdint>

namespace infrastructure::repositories {

    class FileRepository {

            using WriteUnitOfWork = infrastructure::db::sqlite::WriteUnitOfWork;
            using ReadUnitOfWork = infrastructure::db::sqlite::ReadUnitOfWork;
            using UnitOfWork = infrastructure::db::sqlite::UnitOfWork;
            using File = domain::models::File;

        public:

            PersistenceResult<void> create(WriteUnitOfWork& wuov, File file);
            PersistenceResult<void> update(WriteUnitOfWork& wuov, File file);
            PersistenceResult<void> remove(WriteUnitOfWork& wuov, int64_t id);

            PersistenceResult<File> getById(UnitOfWork& uow, int64_t id);
            PersistenceResult<File> getByName(UnitOfWork& uow, const std::string& name);
            PersistenceResult<std::vector<File>> getAll(UnitOfWork& uow);

    };

}
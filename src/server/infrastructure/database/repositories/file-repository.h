#pragma once

#include "infrastructure/database/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileRepository(SqliteDatabase& db)
                : m_db(db) {}

            PersistenceResult<void> create(WriteUnitOfWork& wuov, File file);
            PersistenceResult<void> update(WriteUnitOfWork& wuov, File file);
            PersistenceResult<void> remove(WriteUnitOfWork& wuov, int64_t id);

            PersistenceResult<File> getById(UnitOfWork& uow, int64_t id);
            PersistenceResult<std::vector<File>> getAll(UnitOfWork& uow);

    };

}
#pragma once

#include "infrastructure/database/repositories/persistence-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include "domain/models/file-version.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileVersionRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileVersionRepository(SqliteDatabase& db) : m_db(db) {}

            PersistenceResult<void> addVersion(WriteUnitOfWork& wuov, FileVersion version);
            PersistenceResult<void> removeOldVersions(WriteUnitOfWork& wuov, int64_t fileId, int32_t keepLastN);

            PersistenceResult<std::vector<FileVersion>> getVersions(UnitOfWork& uow, int64_t fileId);
            PersistenceResult<FileVersion> getVersion(UnitOfWork& uow, int64_t fileId, int32_t version);

    };

}
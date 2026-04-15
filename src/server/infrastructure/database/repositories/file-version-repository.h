#pragma once

#include "domain/models/file-version.h"

#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    class FileVersionRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileVersionRepository(SqliteDatabase& db) : m_db(db) {}

            void addVersion(WriteUnitOfWork& wuov, FileVersion version);
            void removeOldVersions(WriteUnitOfWork& wuov, int64_t fileId, int32_t keepLastN);

            std::vector<FileVersion> getVersions(UnitOfWork& uow, int64_t fileId);
            FileVersion getVersion(UnitOfWork& uow, int64_t fileId, int32_t version);

    };

}
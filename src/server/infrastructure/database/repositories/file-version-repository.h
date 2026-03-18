#pragma once

#include "infrastructure/database/models/file-version.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace infrastructure::db::models;

    class FileVersionRepository {

            SqliteDatabase& m_db;

        public:

            explicit FileVersionRepository(SqliteDatabase& db) : m_db(db) {}

            std::future<int64_t> addVersion(FileVersion version);

            std::future<std::vector<FileVersion>> getVersions(int64_t fileId);

            std::future<std::optional<FileVersion>> getVersion(int64_t fileId, int version);

            std::future<void> removeOldVersions(int64_t fileId, int keepLastN);

    };

}
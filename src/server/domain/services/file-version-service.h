#pragma once

#include "domain/models/file-version.h"
#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/file-version-repository.h"

#include <cstdint>
#include <string>
#include <vector>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace infrastructure::repositories;
    using namespace domain::models;

    struct ForkResult {
        int64_t newFileId;
        int64_t newVersionId;
    };

    class FileVersionService {

            sqlite::SqliteDatabase& m_database;
            FileRepository& m_fileRepo;
            FileVersionRepository& m_fileVersionRepo;

        public:

            FileVersionService(sqlite::SqliteDatabase& database,
                               FileRepository& fileRepo,
                               FileVersionRepository& fileVersionRepo) noexcept;

            ServiceResult<int64_t> createNewVersion(int64_t fileId,
                                                    const std::string& logicalNameSnapshot,
                                                    uint64_t physicalKey);

            ServiceResult<FileVersion> getCurrentVersion(int64_t fileId);

            ServiceResult<std::vector<FileVersion>> getAllVersions(int64_t fileId);

    };

}
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "domain/models/file-version.h"
#include "domain/services/service-result.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/file-version-repository.h"

namespace domain::services {

struct ForkResult {
        int64_t newFileId;
        int64_t newVersionId;
};

class FileVersionService {

        using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
        using FileRepository = infrastructure::repositories::FileRepository;
        using FileVersionRepository = infrastructure::repositories::FileVersionRepository;
        using File = domain::models::File;
        using FileVersion = domain::models::FileVersion;

        SqliteDatabase& m_database;
        FileRepository& m_fileRepo;
        FileVersionRepository& m_fileVersionRepo;

    public:

        FileVersionService(SqliteDatabase& database, FileRepository& fileRepo,
                           FileVersionRepository& fileVersionRepo) noexcept;

        ServiceResult<int64_t> createNewVersion(int64_t fileId, const std::string& logicalNameSnapshot,
                                                uint64_t physicalKey);

        ServiceResult<FileVersion> getCurrentVersion(int64_t fileId);

        ServiceResult<FileVersion> getVersionById(int64_t versionId);

        ServiceResult<std::vector<FileVersion>> getAllVersions(int64_t fileId);
};

}  // namespace domain::services
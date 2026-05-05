#pragma once

#include "domain/models/file.h"
#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/file-acl-repository.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/file-version-repository.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-repository.h"

#include <cstdint>
#include <string>
#include <vector>

namespace domain::services {

    class FileService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using FileRepository = infrastructure::repositories::FileRepository;
            using FileVersionRepository = infrastructure::repositories::FileVersionRepository;
            using UserRepository = infrastructure::repositories::UserRepository;
            using GroupRepository = infrastructure::repositories::GroupRepository;
            using FileAclRepository = infrastructure::repositories::FileAclRepository;
            using File = domain::models::File;
            using FileVersion = domain::models::FileVersion;

            static constexpr uint32_t kDefaultMaxVersionCount = 10;

            SqliteDatabase& m_database;
            FileRepository& m_fileRepo;
            FileVersionRepository& m_fileVersionRepo;
            UserRepository& m_userRepo;
            GroupRepository& m_groupRepo;
            FileAclRepository& m_fileAclRepo;

        public:

            FileService(SqliteDatabase& database,
                        FileRepository& fileRepo,
                        FileVersionRepository& fileVersionRepo,
                        UserRepository& userRepo,
                        GroupRepository& groupRepo,
                        FileAclRepository& fileAclRepo) noexcept;

            ServiceResult<int64_t> create(std::string logicalName,
                                          int64_t createdByUser,
                                          uint64_t physicalPath,
                                          uint32_t maxVersionCount = kDefaultMaxVersionCount);

            ServiceResult<File> getById(int64_t fileId);
            ServiceResult<File> getByLogicalName(const std::string& logicalName);
            ServiceResult<std::vector<File>> getAll();

            ServiceResult<void> rename(int64_t fileId, std::string newLogicalName);

            ServiceResult<void> remove(int64_t fileId);

    };

}

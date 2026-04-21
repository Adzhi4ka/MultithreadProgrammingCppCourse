#pragma once

#include "domain/models/file-lock.h"
#include "domain/services/service-result.h"

#include "infrastructure/repositories/file-lock-repository.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace domain::services {

    class FileLockService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using FileLockRepository = infrastructure::repositories::FileLockRepository;
            using FileLock = domain::models::FileLock;

            static constexpr int64_t defaultLockDurationSec = 3600;

            SqliteDatabase& m_database;
            FileLockRepository& m_fileLockRepo;
        
        public:

            FileLockService(SqliteDatabase& database,
                            FileLockRepository& fileLockRepo) noexcept;

            ServiceResult<FileLock> acquireLock(int64_t fileId,
                                                int64_t userId,
                                                int64_t lockDurationSec = defaultLockDurationSec);

            ServiceResult<void> renewLock(int64_t fileId,
                                          int64_t lockToken,
                                          int64_t lockDurationSec = defaultLockDurationSec);

            ServiceResult<void> releaseLock(int64_t fileId, int64_t lockToken);

            ServiceResult<FileLock> getActiveLock(int64_t fileId);

    };

}
#pragma once

#include "domain/models/file-lock.h"
#include "domain/services/service-result.h"

#include "infrastructure/repositories/file-lock-repository.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace infrastructure::repositories;
    using namespace domain::models;

    class FileLockService {

            static constexpr int64_t defaultLockDurationSec = 3600;

            sqlite::SqliteDatabase& m_database;
            FileLockRepository& m_fileLockRepo;
        
        public:

            FileLockService(sqlite::SqliteDatabase& database,
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
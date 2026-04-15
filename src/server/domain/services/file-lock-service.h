#pragma once

#include "domain/models/file-lock.h"

#include "infrastructure/database/repositories/file-lock-repository.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <chrono>
#include <cstdint>
#include <optional>

namespace domain::services {

        using namespace infrastructure::db;
        using namespace domain::models;

    class FileLockService {

            static constexpr int64_t defaultLockDuration = 3600;

            sqlite::SqliteDatabase& m_database;

            repositories::FileLockRepository& m_fileLockRepo;
        
        public:

            bool acquireLock(int64_t fileId, int64_t userId, int64_t lockDuration = defaultLockDuration) {
                auto wouw = m_database.createWriteUnitOfWork();

                m_fileLockRepo.lock(wouw, FileLock{.fileId = fileId,
                                                   .userId = userId,
                                                   .leaseUntil = std::chrono::steady_clock::now().time_since_epoch().count() + defaultLockDuration});
                wouw.commit();

                return true;
            }

            void renewLock(int64_t fileId, int64_t lockDuration = defaultLockDuration) {
                auto wouw = m_database.createWriteUnitOfWork();

                m_fileLockRepo.updateLease(wouw, fileId, std::chrono::steady_clock::now().time_since_epoch().count() + defaultLockDuration);

                wouw.commit();
            }

            void releaseLock(int64_t fileId) {
                auto wouw = m_database.createWriteUnitOfWork();

                m_fileLockRepo.unlock(wouw, fileId);

                wouw.commit();
            }

            std::optional<FileLock> getActiveLock(int64_t fileId) {
                auto rouw = m_database.createReadUnitOfWork();

                return m_fileLockRepo.getLock(rouw, fileId);
            }

    };

};
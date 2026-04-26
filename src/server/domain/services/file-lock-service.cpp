#include "file-lock-service.h"

#include "infrastructure/id-generator/id-generator.h"

namespace {

    using namespace domain::models;

    inline int64_t unixNowSeconds() noexcept {
        using namespace std::chrono;

        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

}

namespace domain::services {

    FileLockService::FileLockService(SqliteDatabase& database,
                                     FileLockRepository& fileLockRepo) noexcept
        : m_database(database),
          m_fileLockRepo(fileLockRepo) {}

    ServiceResult<FileLock> FileLockService::acquireLock(int64_t fileId,
                                                         int64_t userId,
                                                         int64_t lockDurationSec) {

        auto wuow = m_database.createWriteUnitOfWork();

        FileLock lock{.fileId = fileId,
                      .userId = userId,
                      .leaseUntil = unixNowSeconds() + lockDurationSec,
                      .lockToken = infrastructure::id_generator::generateId()};

        auto lockResult = m_fileLockRepo.lock(wuow, lock);
        if (!lockResult) {
            return std::unexpected(mapPersistenceError(lockResult.error()));
        }

        wuow.commit();
        return lock;
    }

    ServiceResult<void> FileLockService::renewLock(int64_t fileId,
                                                   int64_t lockToken,
                                                   int64_t lockDurationSec) {

    /*
        По факту тут есть окно, пока мы пытаемся потом захватить WriteUnitOfWork, в течение которого время может сильно измениться
        Но мне кажется, что это время столь мало, что не так страшно
    */
        {
            auto ruow = m_database.createReadUnitOfWork();

            auto oldLockResult = m_fileLockRepo.getLock(ruow, fileId);
            if (!oldLockResult) {
                return std::unexpected(mapPersistenceError(oldLockResult.error()));
            }

            if (oldLockResult->leaseUntil <= unixNowSeconds()) {
                return std::unexpected(ServiceError::NotFound);
            }

            if (oldLockResult->lockToken != lockToken) {
                return std::unexpected(ServiceError::Forbidden);
            }
        }

        auto wuow = m_database.createWriteUnitOfWork();

        auto renewResult = m_fileLockRepo.updateLease(wuow,
                                                      fileId,
                                                      lockToken,
                                                      unixNowSeconds() + lockDurationSec);

        if (!renewResult) {
            return std::unexpected(mapPersistenceError(renewResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<void> FileLockService::releaseLock(int64_t fileId, int64_t lockToken) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto unlockResult = m_fileLockRepo.unlock(wuow, fileId, lockToken);

        if (!unlockResult) {
            return std::unexpected(mapPersistenceError(unlockResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<FileLock> FileLockService::getActiveLock(int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto lockResult = m_fileLockRepo.getLock(ruow, fileId);
        if (!lockResult) {
            if (lockResult.error() == PersistenceError::NotFound) {
                return std::unexpected(ServiceError::NotFound);
            }

            return std::unexpected(mapPersistenceError(lockResult.error()));
        }

        if (lockResult->leaseUntil <= unixNowSeconds()) {
            return std::unexpected(ServiceError::NotFound);
        }

        return *lockResult;
    }

}
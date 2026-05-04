#include "file-version-service.h"

#include "domain/services/service-result.h"
#include "infrastructure/id-generator/id-generator.h"

#include <chrono>
#include <expected>

namespace {

    using namespace domain::models;

    inline int64_t unixNowSeconds() noexcept {
        using namespace std::chrono;

        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

}

namespace domain::services {

    FileVersionService::FileVersionService(SqliteDatabase& database,
                                           FileRepository& fileRepo,
                                           FileVersionRepository& fileVersionRepo) noexcept
        : m_database(database),
          m_fileRepo(fileRepo),
          m_fileVersionRepo(fileVersionRepo) {}

    ServiceResult<int64_t> FileVersionService::createNewVersion(int64_t fileId,
                                                                const std::string& logicalNameSnapshot,
                                                                uint64_t physicalKey) {

        auto ruow = m_database.createReadUnitOfWork();

        auto fileResult = m_fileRepo.getById(ruow, fileId);
        if (!fileResult) {
            return std::unexpected(mapPersistenceError(fileResult.error()));
        }

        auto versionResult = m_fileVersionRepo.getVersion(ruow, fileResult->currentVersionId);

        if (!versionResult) {
            return std::unexpected(mapPersistenceError(versionResult.error()));
        }

        const int64_t newVersionId = infrastructure::id_generator::generateId();
        const int32_t newVersionNumber = versionResult->version + 1;
        const int64_t createdAt = unixNowSeconds();

        ruow.close();

        auto wuow = m_database.createWriteUnitOfWork();

        auto addVersionResult = m_fileVersionRepo.addVersion(wuow, FileVersion{.id = newVersionId,
                                                                               .fileId = fileId,
                                                                               .version = newVersionNumber,
                                                                               .logicalNameSnapshot = logicalNameSnapshot,
                                                                               .physicalPathName = physicalKey,
                                                                               .createdAt = createdAt});

        if (!addVersionResult) {
            return std::unexpected(mapPersistenceError(addVersionResult.error()));
        }

        fileResult->currentVersionId = newVersionId;

        auto updateFileResult = m_fileRepo.update(wuow, *fileResult);
        if (!updateFileResult) {
            return std::unexpected(mapPersistenceError(updateFileResult.error()));
        }

        auto removeOldResult = m_fileVersionRepo.removeOldVersions(wuow, fileId, fileResult->maxVersionCount);
        if (!removeOldResult) {
            return std::unexpected(mapPersistenceError(removeOldResult.error()));
        }

        wuow.commit();
        return newVersionId;
    }

    ServiceResult<FileVersion> FileVersionService::getCurrentVersion(int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto fileResult = m_fileRepo.getById(ruow, fileId);
        if (!fileResult) {
            return std::unexpected(mapPersistenceError(fileResult.error()));
        }

        auto versionResult = m_fileVersionRepo.getVersion(ruow, fileResult->currentVersionId);
        if (!versionResult) {
            return std::unexpected(mapPersistenceError(versionResult.error()));
        }

        return *versionResult;
    }

    ServiceResult<FileVersion> FileVersionService::getVersionById(int64_t versionId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto versionResult = m_fileVersionRepo.getVersion(ruow, versionId);
        if (!versionResult) {
            return std::unexpected(mapPersistenceError(versionResult.error()));
        }

        return *versionResult;
    }

    ServiceResult<std::vector<FileVersion>> FileVersionService::getAllVersions(int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto versionsResult = m_fileVersionRepo.getVersions(ruow, fileId);
        if (!versionsResult) {
            return std::unexpected(mapPersistenceError(versionsResult.error()));
        }

        if (versionsResult->empty()) {
            return std::unexpected(ServiceError::NotFound);
        }

        return *versionsResult;
    }

}
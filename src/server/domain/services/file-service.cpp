#include "file-service.h"

#include <chrono>

#include "infrastructure/id-generator/id-generator.h"

namespace {

using namespace domain::models;

inline int64_t unixNowSeconds() noexcept {
    using namespace std::chrono;

    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

}  // namespace

namespace domain::services {

FileService::FileService(SqliteDatabase& database, FileRepository& fileRepo, FileVersionRepository& fileVersionRepo,
                         UserRepository& userRepo, GroupRepository& groupRepo, FileAclRepository& fileAclRepo) noexcept
    : m_database(database),
      m_fileRepo(fileRepo),
      m_fileVersionRepo(fileVersionRepo),
      m_userRepo(userRepo),
      m_groupRepo(groupRepo),
      m_fileAclRepo(fileAclRepo) {}

ServiceResult<int64_t> FileService::create(std::string logicalName, int64_t createdByUser, uint64_t physicalPath,
                                           uint32_t maxVersionCount) {

    auto wuow = m_database.createWriteUnitOfWork();

    const int64_t fileId = infrastructure::id_generator::generateId();
    const int64_t versionId = infrastructure::id_generator::generateId();
    const int64_t createdAt = unixNowSeconds();

    auto createFileResult = m_fileRepo.create(wuow, File{.id = fileId,
                                                         .fullLogicalName = logicalName,
                                                         .currentVersionId = versionId,
                                                         .maxVersionCount = maxVersionCount,
                                                         .createdAt = createdAt,
                                                         .createdBy = createdByUser});

    if (!createFileResult) {
        return std::unexpected(mapPersistenceError(createFileResult.error()));
    }

    auto createVersionResult =
        m_fileVersionRepo.addVersion(wuow, FileVersion{.id = versionId,
                                                       .fileId = fileId,
                                                       .version = 1,
                                                       .logicalNameSnapshot = std::move(logicalName),
                                                       .physicalPathName = physicalPath,
                                                       .createdAt = createdAt});

    if (!createVersionResult) {
        return std::unexpected(mapPersistenceError(createVersionResult.error()));
    }

    auto userResult = m_userRepo.getById(wuow, createdByUser);
    if (!userResult) {
        return std::unexpected(mapPersistenceError(userResult.error()));
    }

    auto ownerGroupResult = m_groupRepo.getByName(wuow, userResult->login);
    if (!ownerGroupResult) {
        return std::unexpected(mapPersistenceError(ownerGroupResult.error()));
    }

    auto grantOwnerGroupResult = m_fileAclRepo.grant(
        wuow, FileAcl{.fileId = fileId, .groupId = ownerGroupResult->id, .aclLevel = AclLevel::READ_WRITE});

    if (!grantOwnerGroupResult) {
        return std::unexpected(mapPersistenceError(grantOwnerGroupResult.error()));
    }

    wuow.commit();
    return fileId;
}

ServiceResult<File> FileService::getById(int64_t fileId) {
    auto ruow = m_database.createReadUnitOfWork();

    auto fileResult = m_fileRepo.getById(ruow, fileId);
    if (!fileResult) {
        return std::unexpected(mapPersistenceError(fileResult.error()));
    }

    return *fileResult;
}

ServiceResult<File> FileService::getByLogicalName(const std::string& logicalName) {
    auto ruow = m_database.createReadUnitOfWork();

    auto fileResult = m_fileRepo.getByName(ruow, logicalName);
    if (!fileResult) {
        return std::unexpected(mapPersistenceError(fileResult.error()));
    }

    return *fileResult;
}

ServiceResult<std::vector<File>> FileService::getAll() {
    auto ruow = m_database.createReadUnitOfWork();

    auto filesResult = m_fileRepo.getAll(ruow);
    if (!filesResult) {
        return std::unexpected(mapPersistenceError(filesResult.error()));
    }

    return *filesResult;
}

ServiceResult<void> FileService::rename(int64_t fileId, std::string newLogicalName) {
    auto wuow = m_database.createWriteUnitOfWork();

    auto fileResult = m_fileRepo.getById(wuow, fileId);
    if (!fileResult) {
        return std::unexpected(mapPersistenceError(fileResult.error()));
    }

    auto updateVersionNameResult = m_fileVersionRepo.updateName(wuow, fileResult->currentVersionId, newLogicalName);
    if (!updateVersionNameResult) {
        return std::unexpected(mapPersistenceError(updateVersionNameResult.error()));
    }

    fileResult->fullLogicalName = std::move(newLogicalName);

    auto updateFileResult = m_fileRepo.update(wuow, *fileResult);
    if (!updateFileResult) {
        return std::unexpected(mapPersistenceError(updateFileResult.error()));
    }

    wuow.commit();
    return {};
}

ServiceResult<void> FileService::remove(int64_t fileId) {
    auto wuow = m_database.createWriteUnitOfWork();

    auto removeResult = m_fileRepo.remove(wuow, fileId);
    if (!removeResult) {
        return std::unexpected(mapPersistenceError(removeResult.error()));
    }

    wuow.commit();
    return {};
}

}  // namespace domain::services
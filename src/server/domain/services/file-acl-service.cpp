#include "file-acl-service.h"

namespace {

    using namespace domain::models;

}

namespace domain::services {

    FileAclService::FileAclService(SqliteDatabase& database,
                                   FileAclRepository& fileAclRepo,
                                   UserGroupRepository& userGroupRepo) noexcept
        : m_database(database),
          m_fileAclRepo(fileAclRepo),
          m_userGroupRepo(userGroupRepo) {}

    ServiceResult<AclLevel> FileAclService::getGroupAclLevel(int64_t groupId, int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto aclResult = m_fileAclRepo.getFileAcl(ruow, fileId, groupId);
        if (aclResult) {
            return *aclResult;
        }

        if (aclResult.error() == PersistenceError::NotFound) {
            return AclLevel::NO_PROPERTY;
        }

        return std::unexpected(mapPersistenceError(aclResult.error()));
    }

    ServiceResult<AclLevel> FileAclService::getUserAclLevel(int64_t userId, int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto groupIdsResult = m_userGroupRepo.getGroupIdsOfUser(ruow, userId);
        if (!groupIdsResult) {
            return std::unexpected(mapPersistenceError(groupIdsResult.error()));
        }

        AclLevel bestAcl = AclLevel::NO_PROPERTY;

        for (const auto groupId : *groupIdsResult) {
            auto aclResult = m_fileAclRepo.getFileAcl(ruow, fileId, groupId);

            if (!aclResult) {
                if (aclResult.error() == PersistenceError::NotFound) {
                    continue;
                }

                return std::unexpected(mapPersistenceError(aclResult.error()));
            }

            if (*aclResult > bestAcl) {
                bestAcl = *aclResult;
            }

            if (bestAcl == AclLevel::READ_WRITE) {
                return bestAcl;
            }
        }

        return bestAcl;
    }

    ServiceResult<void> FileAclService::setGroupAclLevel(int64_t fileId,
                                                         int64_t groupId,
                                                         AclLevel aclLevel) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto setResult = m_fileAclRepo.grant(wuow, FileAcl{.fileId = fileId,
                                                           .groupId = groupId,
                                                           .aclLevel = aclLevel});

        if (!setResult) {
            return std::unexpected(mapPersistenceError(setResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<void> FileAclService::removeGroupAclLevel(int64_t fileId, int64_t groupId) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto revokeResult = m_fileAclRepo.revoke(wuow, fileId, groupId);
        if (!revokeResult) {
            return std::unexpected(mapPersistenceError(revokeResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<std::vector<FileAcl>> FileAclService::getFileAcls(int64_t fileId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto result = m_fileAclRepo.getFileAclsToFileId(ruow, fileId);
        if (!result) {
            return std::unexpected(mapPersistenceError(result.error()));
        }

        return *result;
    }

    ServiceResult<std::vector<FileAcl>> FileAclService::getGroupAcls(int64_t groupId) {
        auto ruow = m_database.createReadUnitOfWork();

        auto result = m_fileAclRepo.getGroupFileAcls(ruow, groupId);
        if (!result) {
            return std::unexpected(mapPersistenceError(result.error()));
        }

        return *result;
    }

}
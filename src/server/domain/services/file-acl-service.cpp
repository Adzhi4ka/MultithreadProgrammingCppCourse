#include "file-acl-service.h"

namespace domain::services {

    using namespace infrastructure;
    using namespace infrastructure::repositories;
    using namespace domain::models;

    FileAclService::FileAclService(db::sqlite::SqliteDatabase& database,
                                   FileAclRepository& fileAclRepo,
                                   UserGroupRepository& userGroupRepo) noexcept
        : m_database(database),
          m_fileAclRepo(fileAclRepo),
          m_userGroupRepo(userGroupRepo) {}

    ServiceResult<AclLevel> FileAclService::getGroupAclLevel(int64_t groupId, int64_t fileId) {
        auto uow = m_database.createReadUnitOfWork();

        auto aclResult = m_fileAclRepo.getFileAcl(uow, fileId, groupId);

        if (aclResult) {
            return *aclResult;
        }

        if (aclResult.error() == PersistenceError::NotFound) {
            return AclLevel::NO_PROPERTY;
        }

        return std::unexpected(mapPersistenceError(aclResult.error()));
    }

    ServiceResult<AclLevel> FileAclService::getUserAclLevel(int64_t userId, int64_t fileId) {
        auto uow = m_database.createReadUnitOfWork();

        auto userGroupIdsResult = m_userGroupRepo.getGroupIdsOfUser(uow, userId);
        if (!userGroupIdsResult) {
            return std::unexpected(mapPersistenceError(userGroupIdsResult.error()));
        }

        AclLevel userAclLevel = models::AclLevel::NO_PROPERTY;

        for (const auto groupId : *userGroupIdsResult) {
            auto aclResult = m_fileAclRepo.getFileAcl(uow, fileId, groupId);

            if (!aclResult) {
                if (aclResult.error() == PersistenceError::NotFound) {
                    continue;
                }

                return std::unexpected(mapPersistenceError(aclResult.error()));
            }

            auto aclLevel = *aclResult;

            if (aclLevel > userAclLevel) {
                userAclLevel = aclLevel;
            }

            if (userAclLevel == AclLevel::READ_WRITE) {
                return userAclLevel;
            }
        }

        return userAclLevel;
    }

    ServiceResult<void> FileAclService::createGroupAclLevel(int64_t fileId, int64_t groupId, AclLevel aclLevel) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto grantResult = m_fileAclRepo.grant(wuow, FileAcl{.fileId = fileId,
                                                             .groupId = groupId,
                                                             .aclLevel = aclLevel});

        if (!grantResult) {
            return std::unexpected(mapPersistenceError(grantResult.error()));
        }

        wuow.commit();
        return {};
    }

    ServiceResult<void> FileAclService::removeGroupAclLevel(uint64_t fileId, uint64_t groupId) {
        auto wuow = m_database.createWriteUnitOfWork();

        auto revokeResult = m_fileAclRepo.revoke(wuow, fileId, groupId);
        if (!revokeResult) {
            return std::unexpected(mapPersistenceError(revokeResult.error()));
        }

        wuow.commit();
        return {};
    }

};
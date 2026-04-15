#pragma once

#include "domain/models/file-acl.h"

#include "infrastructure/database/repositories/file-acl-repository.h"
#include "infrastructure/database/repositories/user-group-repository.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace domain::models;

    class FileAclService {

            sqlite::SqliteDatabase& m_database;

            repositories::FileAclRepository& m_fileAclRepo;

            repositories::UserGroupRepository& m_userGroupRepo;
        
        public:

            AclLevel getGroupAclLevel(int64_t groupId, int64_t fileId) {
                auto uow = m_database.createReadUnitOfWork();

                return m_fileAclRepo.getFileAcl(uow, fileId, groupId);
            }

            AclLevel getUserAclLevel(int64_t userId, int64_t fileId) {
                auto uow = m_database.createReadUnitOfWork();

                AclLevel userAclLevel = models::AclLevel::NO_PROPERTY;

                const auto userGroupIds = m_userGroupRepo.getGroupIdsOfUser(uow, userId);

                for (const auto groupId : userGroupIds) {
                    auto aclLevel = m_fileAclRepo.getFileAcl(uow, fileId, groupId);
                    if (aclLevel > userAclLevel) {
                        userAclLevel = aclLevel;
                    }

                    if (userAclLevel == AclLevel::READ_WRITE) {
                        return userAclLevel;
                    }
                }

                return userAclLevel;
            }

            void createGroupAclLevel(int64_t fileId, int64_t groupId, AclLevel aclLevel) {
                auto wuow = m_database.createWriteUnitOfWork();
                m_fileAclRepo.grant(wuow, FileAcl{.fileId = fileId, .groupId = groupId, .aclLevel = aclLevel});
                wuow.commit();
            }

            void removeGroupAclLevel(uint64_t fileId, uint64_t groupId) {
                auto wuow = m_database.createWriteUnitOfWork();
                m_fileAclRepo.revoke(wuow, fileId, groupId);
                wuow.commit();
            }

    };

};
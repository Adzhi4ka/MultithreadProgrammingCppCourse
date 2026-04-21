#pragma once

#include "domain/models/file-acl.h"
#include "domain/services/service-result.h"

#include "infrastructure/repositories/file-acl-repository.h"
#include "infrastructure/repositories/user-group-repository.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <cstdint>

namespace domain::services {

    class FileAclService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using FileAclRepository = infrastructure::repositories::FileAclRepository;
            using UserGroupRepository = infrastructure::repositories::UserGroupRepository;
            using AclLevel = domain::models::AclLevel;

            SqliteDatabase& m_database;

            FileAclRepository& m_fileAclRepo;
            UserGroupRepository& m_userGroupRepo;
        
        public:

            FileAclService(SqliteDatabase& database,
                           FileAclRepository& fileAclRepo,
                           UserGroupRepository& userGroupRepo) noexcept;

            ServiceResult<AclLevel> getGroupAclLevel(int64_t groupId, int64_t fileId);

            ServiceResult<AclLevel> getUserAclLevel(int64_t userId, int64_t fileId);

            ServiceResult<void> createGroupAclLevel(int64_t fileId, int64_t groupId, AclLevel aclLevel);

            ServiceResult<void> removeGroupAclLevel(uint64_t fileId, uint64_t groupId);

    };

};
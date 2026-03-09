#pragma once

#include "infrastructure/database/models/file-acl.h"
#include "infrastructure/database/models/file-lock.h"
#include "infrastructure/database/models/file.h"
#include "infrastructure/database/models/group.h"
#include "infrastructure/database/models/user.h"

#include <string>

#include <sqlite_orm/sqlite_orm.h>

namespace infrastructure::db::sqlite {

    inline auto makeStorage(const std::string& path) {

        using namespace sqlite_orm;

        return make_storage(
            path,
            // Users
            sqlite_orm::make_table(
                "users",
                make_column("id", &models::User::id, primary_key()),
                make_column("login", &models::User::login),
                make_column("password_hash", &models::User::passwordHash)
            ),
            // Groups
            make_table(
                "groups",
                make_column("id", &models::Group::id, primary_key()),
                make_column("name", &models::Group::name)
            ),
            // Files
            make_table(
                "files",
                make_column("id", &models::File::id, primary_key()),
                make_column("name", &models::File::name),
                make_column("version", &models::File::version),
                make_column("created_at", &models::File::created_at),
                make_column("modified_at", &models::File::modified_at),
                make_column("modified_by", &models::File::modified_by)
            ),
            // FileAcl
            make_table(
                "file_acl",
                make_column("file_id", &models::FileAcl::fileId),
                make_column("group_id", &models::FileAcl::groupId),
                make_column("acl_level", &models::FileAcl::aclLevel)
            ),
            // FileLock
            make_table(
                "file_lock",
                make_column("file_id", &models::FileLock::fileId),
                make_column("user_id", &models::FileLock::userId),
                make_column("lease_until", &models::FileLock::leaseUntil)
            )
        );
    }

}
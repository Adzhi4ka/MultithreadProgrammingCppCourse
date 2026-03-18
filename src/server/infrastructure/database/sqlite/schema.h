#pragma once

#include "infrastructure/database/models/file-lock.h"
#include "infrastructure/database/models/file-acl.h"
#include "infrastructure/database/models/file-version.h"
#include "infrastructure/database/models/file.h"
#include "infrastructure/database/models/group.h"
#include "infrastructure/database/models/user.h"
#include "infrastructure/database/models/user-group.h"

#include <string>
#include <sqlite_orm/sqlite_orm.h>

namespace infrastructure::db::sqlite {

    inline auto makeStorage(const std::string& path) {
        using namespace sqlite_orm;

        return make_storage(
            path,

            // Users
            make_table(
                "users",
                make_column("id", &models::User::id),
                make_column("login", &models::User::login),
                make_column("password_hash", &models::User::passwordHash),
                primary_key(&models::User::id),
                unique(&models::User::login)
            ),

            // Groups
            make_table(
                "groups",
                make_column("id", &models::Group::id),
                make_column("name", &models::Group::name),
                primary_key(&models::Group::id),
                unique(&models::Group::name)
            ),

            // User <-> Group
            make_table(
                "user_group",
                make_column("user_id", &models::UserGroup::userId),
                make_column("group_id", &models::UserGroup::groupId),
                primary_key(&models::UserGroup::userId, &models::UserGroup::groupId),
                foreign_key(&models::UserGroup::userId).references(&models::User::id).on_delete.cascade(),
                foreign_key(&models::UserGroup::groupId).references(&models::Group::id).on_delete.cascade()
            ),

            // Files
            make_table(
                "files",
                make_column("id", &models::File::id),
                make_column("full_logical_name", &models::File::fullLogicalName),
                make_column("current_version_id", &models::File::currentVersionId),
                make_column("max_version_count", &models::File::maxVersionCount),
                make_column("created_at", &models::File::createdAt),
                make_column("created_by", &models::File::createdBy),
                primary_key(&models::File::id),
                unique(&models::File::fullLogicalName),
                foreign_key(&models::File::createdBy).references(&models::User::id)
            ),

            // FileVersions
            make_table(
                "file_versions",
                make_column("id", &models::FileVersion::id),
                make_column("file_id", &models::FileVersion::fileId),
                make_column("version", &models::FileVersion::version),
                make_column("logical_name_snapshot", &models::FileVersion::logicalNameSnapshot),
                make_column("physical_path_name", &models::FileVersion::physicalPathName),
                make_column("created_at", &models::FileVersion::createdAt),
                primary_key(&models::FileVersion::id),
                foreign_key(&models::FileVersion::fileId).references(&models::File::id).on_delete.cascade(),
                unique(&models::FileVersion::fileId, &models::FileVersion::version)
            ),

            // FileAcl
            make_table(
                "file_acl",
                make_column("file_id", &models::FileAcl::fileId),
                make_column("group_id", &models::FileAcl::groupId),
                make_column("acl_level", &models::FileAcl::aclLevel),
                primary_key(&models::FileAcl::fileId, &models::FileAcl::groupId),
                foreign_key(&models::FileAcl::fileId).references(&models::File::id).on_delete.cascade(),
                foreign_key(&models::FileAcl::groupId).references(&models::Group::id)
            ),

            // FileLock
            make_table(
                "file_lock",
                make_column("file_id", &models::FileLock::fileId),
                make_column("user_id", &models::FileLock::userId),
                make_column("lease_until", &models::FileLock::leaseUntil),
                primary_key(&models::FileLock::fileId),
                foreign_key(&models::FileLock::fileId).references(&models::File::id).on_delete.cascade(),
                foreign_key(&models::FileLock::userId).references(&models::User::id)
            )
        );
    }

} // namespace infrastructure::db::sqlite
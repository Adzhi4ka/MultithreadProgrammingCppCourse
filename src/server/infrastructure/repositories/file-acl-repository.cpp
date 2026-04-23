#include "file-acl-repository.h"

namespace {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileAcl readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileAcl{
            .fileId = stmt.getColumn(readIndex++).getInt64(),
            .groupId = stmt.getColumn(readIndex++).getInt64(),
            .aclLevel = static_cast<AclLevel>(stmt.getColumn(readIndex++).getInt())
        };
    }

}

namespace infrastructure::repositories {

    PersistenceResult<void> FileAclRepository::grant(WriteUnitOfWork& wuow, FileAcl fileAcl) {
        constexpr const char* const sql = {
            "INSERT INTO file_acl "
                "(file_id, group_id, acl_level) "
            "VALUES (?, ?, ?) "
            "ON CONFLICT(file_id, group_id) DO UPDATE SET "
                "acl_level = excluded.acl_level;"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileAcl.fileId);
                statement.bind(bindIndex++, fileAcl.groupId);
                statement.bind(bindIndex++, static_cast<int32_t>(fileAcl.aclLevel));
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileAclRepository::revoke(WriteUnitOfWork& wuow, int64_t fileId, int64_t groupId) {
        constexpr const char* const sql = {
            "DELETE FROM file_acl "
            "WHERE file_id = ? AND group_id = ?;"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, groupId);
            }

            statement.exec();

            if (wuow.connection().getChanges() == 0) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<AclLevel> FileAclRepository::getFileAcl(UnitOfWork& uow, int64_t fileId, int64_t groupId) {
        constexpr const char* const sql = {
            "SELECT acl_level "
            "FROM file_acl "
            "WHERE file_id = ? AND group_id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, groupId);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return static_cast<AclLevel>(statement.getColumn(0).getInt());
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<FileAcl>> FileAclRepository::getFileAclsToFileId(UnitOfWork& uow, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT file_id, group_id, acl_level "
            "FROM file_acl "
            "WHERE file_id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
            }

            std::vector<FileAcl> fileAcls;
            while (statement.executeStep()) {
                fileAcls.emplace_back(readFromStatement(statement));
            }

            return fileAcls;
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<FileAcl>> FileAclRepository::getGroupFileAcls(UnitOfWork& uow, int64_t groupId) {
        constexpr const char* const sql = {
            "SELECT file_id, group_id, acl_level "
            "FROM file_acl "
            "WHERE group_id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, groupId);
            }

            std::vector<FileAcl> fileAcls;
            while (statement.executeStep()) {
                fileAcls.emplace_back(readFromStatement(statement));
            }

            return fileAcls;
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

}
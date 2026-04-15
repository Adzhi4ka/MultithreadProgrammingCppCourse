#include "file-acl-repository.h"

#include <cstdint>
#include <expected>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileAcl readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileAcl{.fileId = stmt.getColumn(readIndex++),
                       .groupId = stmt.getColumn(readIndex++),
                       .aclLevel = (AclLevel)stmt.getColumn(readIndex++).getInt()};
    }

    RepositoryOpResult<void> FileAclRepository::grant(WriteUnitOfWork& wuov, FileAcl fileAcl) {
        constexpr const char* const sql = {
            "INSERT INTO file_acl "
                "(file_id, group_id, acl_level) "
            "VALUES (?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileAcl.fileId);
                statement.bind(bindIndex++, fileAcl.groupId);
                statement.bind(bindIndex++, (int32_t)fileAcl.aclLevel);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }

    RepositoryOpResult<void> FileAclRepository::revoke(WriteUnitOfWork& wuov, int64_t fileId, int64_t groupId) {
        constexpr const char* const sql = {
            "DELETE FROM file_acl "
            "WHERE file_id = ? AND group_id = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, groupId);
            }

            statement.executeStep();
            return {};
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }

    RepositoryOpResult<AclLevel> FileAclRepository::getFileAcl(UnitOfWork& uow, int64_t fileId, int64_t groupId) {
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
                return std::unexpected(RepositoryError::NotFound);
            }

            return (AclLevel)statement.getColumn(0).getInt();
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }

    RepositoryOpResult<std::vector<FileAcl>> FileAclRepository::getFileAclsToFileId(UnitOfWork& uow, int64_t fileId) {
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
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }

    RepositoryOpResult<std::vector<FileAcl>> FileAclRepository::getGroupFileAcls(UnitOfWork& uow, int64_t groupId) {
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
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }
}
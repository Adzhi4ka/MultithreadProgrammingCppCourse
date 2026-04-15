#include "file-acl-repository.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileAcl readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileAcl{.fileId = stmt.getColumn(readIndex++),
                       .groupId = stmt.getColumn(readIndex++),
                       .aclLevel = (AclLevel)stmt.getColumn(readIndex++).getInt()};
    }

    void FileAclRepository::grant(WriteUnitOfWork& wuov, FileAcl fileAcl) {
        constexpr const char* const sql = {
            "INSERT INTO file_acl "
                "(file_id, group_id, acl_level) "
            "VALUES (?, ?, ?);"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileAcl.fileId);
            statement.bind(bindIndex++, fileAcl.groupId);
            statement.bind(bindIndex++, (int32_t)fileAcl.aclLevel);
        }

        statement.executeStep();
    }

    void FileAclRepository::revoke(WriteUnitOfWork& wuov, int64_t fileId, int64_t groupId) {
        constexpr const char* const sql = {
            "DELETE FROM file_acl "
            "WHERE file_id = ? AND group_id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
            statement.bind(bindIndex++, groupId);
        }

        statement.executeStep();
    }

    std::vector<FileAcl> FileAclRepository::getFileAcl(UnitOfWork& uov, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT file_id, group_id, acl_level "
            "FROM file_acl "
            "WHERE file_id = ?;"
        };

        SQLite::Statement statement(uov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
        }

        std::vector<FileAcl> fileAcls;
        while (statement.executeStep()) {
            fileAcls.emplace_back(readFromStatement(statement));
        }

        return fileAcls;
    }

    std::vector<FileAcl> FileAclRepository::getGroupAcl(UnitOfWork& uov, int64_t groupId) {
        constexpr const char* const sql = {
            "SELECT file_id, group_id, acl_level "
            "FROM file_acl "
            "WHERE group_id = ?;"
        };

        SQLite::Statement statement(uov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, groupId);
        }

        std::vector<FileAcl> fileAcls;
        while (statement.executeStep()) {
            fileAcls.emplace_back(readFromStatement(statement));
        }

        return fileAcls;
    }
}
#include "file-version-repository.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileVersion readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileVersion{.id = stmt.getColumn(readIndex++),
                           .fileId = stmt.getColumn(readIndex++),
                           .version = stmt.getColumn(readIndex++),
                           .logicalNameSnapshot = stmt.getColumn(readIndex++),
                           .physicalPathName = (uint64_t)stmt.getColumn(readIndex++).getInt64(),
                           .createdAt = stmt.getColumn(readIndex++)};
    }

    void FileVersionRepository::addVersion(WriteUnitOfWork& wuov, FileVersion version) {
        constexpr const char* const sql = {
            "INSERT INTO file_versions "
                "(id, file_id, version, logical_name_snapshot, physical_path_name, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?);"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, version.id);
            statement.bind(bindIndex++, version.fileId);
            statement.bind(bindIndex++, version.version);
            statement.bindNoCopy(bindIndex++, version.logicalNameSnapshot);
            statement.bind(bindIndex++, (int64_t)version.physicalPathName);
            statement.bind(bindIndex++, version.createdAt);
        }

        statement.executeStep();
    }

    void FileVersionRepository::removeOldVersions(WriteUnitOfWork& wuov, int64_t fileId, int keepLastN) {
        constexpr const char* const sql = {
            "DELETE FROM file_versions "
            "WHERE file_id = ?1 "
            "AND id NOT IN ("
                "SELECT id "
                "FROM file_versions "
                "WHERE file_id = ?1 "
                "ORDER BY version DESC "
                "LIMIT ?2"
            ");"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
            statement.bind(bindIndex++, keepLastN);
        }

        statement.executeStep();
    }

    std::vector<FileVersion> FileVersionRepository::getVersions(UnitOfWork& uow, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT id, file_id, version, logical_name_snapshot, physical_path_name, created_at "
            "FROM file_versions "
            "WHERE file_id = ? "
            "ORDER BY version ASC;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
        }

        std::vector<FileVersion> fileVersions;

        while (statement.executeStep()) {
            fileVersions.emplace_back(readFromStatement(statement));
        }

        return fileVersions;
    }

    FileVersion FileVersionRepository::getVersion(UnitOfWork& uow, int64_t fileId, int32_t version) {
        constexpr const char* const sql = {
            "SELECT id, file_id, version, logical_name_snapshot, physical_path_name, created_at "
            "FROM file_versions "
            "WHERE file_id = ? AND version = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
            statement.bind(bindIndex++, version);
        }

        statement.executeStep();

        return readFromStatement(statement);
    }

}
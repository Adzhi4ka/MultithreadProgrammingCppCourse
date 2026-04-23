#include "file-version-repository.h"

namespace {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileVersion readFromStatement(SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileVersion{
            .id = stmt.getColumn(readIndex++),
            .fileId = stmt.getColumn(readIndex++),
            .version = stmt.getColumn(readIndex++),
            .logicalNameSnapshot = stmt.getColumn(readIndex++),
            .physicalPathName = (uint64_t)stmt.getColumn(readIndex++).getInt64(),
            .createdAt = stmt.getColumn(readIndex++)
        };
    }

}

namespace infrastructure::repositories {

    PersistenceResult<void> FileVersionRepository::addVersion(WriteUnitOfWork& wuow, FileVersion version) {
        constexpr const char* const sql = {
            "INSERT INTO file_versions "
                "(id, file_id, version, logical_name_snapshot, physical_path_name, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, version.id);
                statement.bind(bindIndex++, version.fileId);
                statement.bind(bindIndex++, version.version);
                statement.bindNoCopy(bindIndex++, version.logicalNameSnapshot);
                statement.bind(bindIndex++, (int64_t)version.physicalPathName);
                statement.bind(bindIndex++, version.createdAt);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileVersionRepository::updateName(WriteUnitOfWork& wuow, int64_t versionId, const std::string& name) {
        constexpr const char* const sql = {
            "UPDATE file_versions "
            "SET logical_name_snapshot = ? "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(wuow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bindNoCopy(bindIndex++, name);
                statement.bind(bindIndex++, versionId);
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

    PersistenceResult<void> FileVersionRepository::removeOldVersions(WriteUnitOfWork& wuow, int64_t fileId, int keepLastN) {
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

        try {
            SQLite::Statement statement(wuow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, keepLastN);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<FileVersion>> FileVersionRepository::getVersions(UnitOfWork& uow, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT id, file_id, version, logical_name_snapshot, physical_path_name, created_at "
            "FROM file_versions "
            "WHERE file_id = ? "
            "ORDER BY version ASC;"
        };

        try {
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
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<FileVersion> FileVersionRepository::getVersion(UnitOfWork& uow, int64_t versionId) {
        constexpr const char* const sql = {
            "SELECT id, file_id, version, logical_name_snapshot, physical_path_name, created_at "
            "FROM file_versions "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, versionId);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return readFromStatement(statement);
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<int64_t> FileVersionRepository::getVersionId(UnitOfWork& uow, int64_t fileId, int32_t version) {
        constexpr const char* const sql = {
            "SELECT id "
            "FROM file_versions "
            "WHERE file_id = ? AND version = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, version);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return statement.getColumn(0).getInt64();
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

}
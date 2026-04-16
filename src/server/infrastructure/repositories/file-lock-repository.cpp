#include "file-lock-repository.h"

namespace infrastructure::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline FileLock readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return FileLock{
            .fileId = stmt.getColumn(readIndex++).getInt64(),
            .userId = stmt.getColumn(readIndex++).getInt64(),
            .leaseUntil = stmt.getColumn(readIndex++).getInt64(),
            .lockToken = stmt.getColumn(readIndex++).getInt64()
        };
    }

    PersistenceResult<void> FileLockRepository::lock(WriteUnitOfWork& wuov, FileLock file) {
        constexpr const char* const sql = {
            "INSERT INTO file_lock "
                "(file_id, user_id, lease_until, lock_token) "
            "VALUES (?, ?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, file.fileId);
                statement.bind(bindIndex++, file.userId);
                statement.bind(bindIndex++, file.leaseUntil);
                statement.bind(bindIndex++, file.lockToken);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileLockRepository::unlock(WriteUnitOfWork& wuov, int64_t fileId, int64_t lockToken) {
        constexpr const char* const sql = {
            "DELETE FROM file_lock "
            "WHERE file_id = ? AND lock_token = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, lockToken);
            }

            statement.exec();

            if (wuov.connection().getChanges() == 0) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<FileLock> FileLockRepository::getLock(UnitOfWork& uow, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT file_id, user_id, lease_until, lock_token "
            "FROM file_lock "
            "WHERE file_id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, fileId);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return readFromStatement(statement);
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileLockRepository::updateLease(
        WriteUnitOfWork& wuov,
        int64_t fileId,
        int64_t lockToken,
        int64_t leaseUntil
    ) {
        constexpr const char* const sql = {
            "UPDATE file_lock "
            "SET lease_until = ? "
            "WHERE file_id = ? AND lock_token = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, leaseUntil);
                statement.bind(bindIndex++, fileId);
                statement.bind(bindIndex++, lockToken);
            }

            statement.exec();

            if (wuov.connection().getChanges() == 0) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

}
#include "file-lock-repository.h"
#include "SQLiteCpp/Statement.h"
#include "domain/models/file-lock.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    RepositoryOpResult<void> FileLockRepository::lock(WriteUnitOfWork& wuov, FileLock file) {
        constexpr const char* const sql = {
            "INSERT INTO file_lock"
                "(file_id, user_id, lease_until) "
            "VALUES (?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, file.fileId);
                statement.bind(bindIndex++, file.userId);
                statement.bind(bindIndex++, file.leaseUntil);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception&) {
            return std::unexpected(RepositoryError::InternalError);
        }
    }

    RepositoryOpResult<void> FileLockRepository::unlock(WriteUnitOfWork& wuov, int64_t fileId) {
        constexpr const char* const sql = {
            "DELETE FROM file_lock "
            "WHERE file_id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
        }

        statement.executeStep();
    }

    RepositoryOpResult<FileLock> FileLockRepository::getLock(UnitOfWork& uow, int64_t fileId) {
        constexpr const char* const sql = {
            "SELECT file_id, user_id, lease_until "
            "FROM file_lock "
            "WHERE file_id = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, fileId);
        }

        statement.executeStep();
        
        int readIndex = 0;

        return FileLock{.fileId = statement.getColumn(readIndex++),
                        .userId = statement.getColumn(readIndex++),
                        .leaseUntil = statement.getColumn(readIndex++)};
    }

    RepositoryOpResult<void> FileLockRepository::updateLease(WriteUnitOfWork& wuov, int64_t fileId, int64_t leaseUntil) {
        constexpr const char* const sql = {
            "UPDATE file_lock "
            "SET lease_until = ? "
            "WHERE file_id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);
 
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, leaseUntil);
            statement.bind(bindIndex++, fileId);
        }

        statement.executeStep();
    }

}
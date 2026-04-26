#include "file-repository.h"

namespace {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline File readFromStatement(SQLite::Statement& stmt) {
        int readIndex = 0;
        return File{
            .id = stmt.getColumn(readIndex++),
            .fullLogicalName = stmt.getColumn(readIndex++),
            .currentVersionId = stmt.getColumn(readIndex++),
            .maxVersionCount = stmt.getColumn(readIndex++),
            .createdAt = stmt.getColumn(readIndex++),
            .createdBy = stmt.getColumn(readIndex++)
        };
    }

}

namespace infrastructure::repositories {

    PersistenceResult<File> FileRepository::getById(UnitOfWork& uow, int64_t id) {
        constexpr const char* const sql = {
            "SELECT id, full_logical_name, current_version_id, max_version_count, created_at, created_by "
            "FROM files "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, id);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return readFromStatement(statement);
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<File> FileRepository::getByName(UnitOfWork& uow, const std::string& name) {
        constexpr const char* const sql = {
            "SELECT id, full_logical_name, current_version_id, max_version_count, created_at, created_by "
            "FROM files "
            "WHERE full_logical_name = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bindNoCopy(bindIndex++, name);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return readFromStatement(statement);
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<File>> FileRepository::getAll(UnitOfWork& uow) {
        constexpr const char* const sql = {
            "SELECT id, full_logical_name, current_version_id, max_version_count, created_at, created_by "
            "FROM files;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);

            std::vector<File> files;

            while (statement.executeStep()) {
                files.emplace_back(readFromStatement(statement));
            }

            return files;
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileRepository::create(WriteUnitOfWork& wuov, File file) {
        constexpr const char* const sql = {
            "INSERT INTO files "
                "(id, full_logical_name, current_version_id, max_version_count, created_at, created_by) "
            "VALUES (?, ?, ?, ?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);

            {
                int bindIndex = 1;
                statement.bind(bindIndex++, file.id);
                statement.bindNoCopy(bindIndex++, file.fullLogicalName);
                statement.bind(bindIndex++, file.currentVersionId);
                statement.bind(bindIndex++, file.maxVersionCount);
                statement.bind(bindIndex++, file.createdAt);
                statement.bind(bindIndex++, file.createdBy);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> FileRepository::update(WriteUnitOfWork& wuov, File file) {
        constexpr const char* const sql = {
            "UPDATE files "
            "SET full_logical_name = ?, "
                "current_version_id = ?, "
                "max_version_count = ?, "
                "created_at = ?, "
                "created_by = ? "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);

            {
                int bindIndex = 1;
                statement.bindNoCopy(bindIndex++, file.fullLogicalName);
                statement.bind(bindIndex++, file.currentVersionId);
                statement.bind(bindIndex++, file.maxVersionCount);
                statement.bind(bindIndex++, file.createdAt);
                statement.bind(bindIndex++, file.createdBy);
                statement.bind(bindIndex++, file.id);
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

    PersistenceResult<void> FileRepository::remove(WriteUnitOfWork& wuov, int64_t id) {
        constexpr const char* const sql = {
            "DELETE FROM files "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);

            {
                int bindIndex = 1;
                statement.bind(bindIndex++, id);
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

} // namespace infrastructure::repositories
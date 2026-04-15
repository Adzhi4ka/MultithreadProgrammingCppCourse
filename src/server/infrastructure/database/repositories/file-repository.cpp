#include "file-repository.h"
#include "domain/models/file.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline File readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return File{.id = stmt.getColumn(readIndex++),
                    .fullLogicalName = stmt.getColumn(readIndex++),
                    .currentVersionId = stmt.getColumn(readIndex++),
                    .maxVersionCount = stmt.getColumn(readIndex++),
                    .createdAt = stmt.getColumn(readIndex++),
                    .createdBy = stmt.getColumn(readIndex++)};
    }

    File FileRepository::getById(UnitOfWork& uow, int64_t id) {
        constexpr const char* const sql = {
            "SELECT id, full_logical_name, current_version_id, max_version_count, created_at, created_by "
            "FROM files "
            "WHERE id = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, id);
        }

        statement.executeStep();

        return readFromStatement(statement);
    }

    std::vector<File> FileRepository::getAll(UnitOfWork& uow) {
        constexpr const char* const sql = {
            "SELECT id, full_logical_name, current_version_id, max_version_count, created_at, created_by "
            "FROM files;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        std::vector<File> files;

        while (statement.executeStep()) {
            files.emplace_back(readFromStatement(statement));
        }

        return files;
    }

    void FileRepository::create(WriteUnitOfWork& wuov, File file) {
        constexpr const char* const sql = {
            "INSERT INTO files "
                "(id, full_logical_name, current_version_id, max_version_count, created_at, created_by) "
            "VALUES (?, ?, ?, ?, ?, ?);"
        };

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

        statement.executeStep();
    }

    void FileRepository::update(WriteUnitOfWork& wuov, File file) {
        constexpr const char* const sql = {
            "UPDATE files "
            "SET full_logical_name = ?, "
                "current_version_id = ?, "
                "max_version_count = ?, "
                "created_at = ?, "
                "created_by = ? "
            "WHERE id = ?;"
        };

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

        statement.executeStep();
    }

    void FileRepository::remove(WriteUnitOfWork& wuov, int64_t id) {
        constexpr const char* const sql = {
            "DELETE FROM files "
            "WHERE id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, id);
        }

        statement.executeStep();
    }

} // namespace infrastructure::db::repositories
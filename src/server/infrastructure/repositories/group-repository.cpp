#include "group-repository.h"

namespace {

using namespace infrastructure::db::sqlite;
using namespace domain::models;

inline Group readFromStatement(SQLite::Statement& stmt) {
    int readIndex = 0;
    return Group{.id = stmt.getColumn(readIndex++), .name = stmt.getColumn(readIndex++)};
}

}  // namespace

namespace infrastructure::repositories {

PersistenceResult<void> GroupRepository::create(WriteUnitOfWork& wuow, Group group) {
    constexpr const char* const sql = {
        "INSERT INTO groups "
        "(id, name) "
        "VALUES (?, ?);"};

    try {
        SQLite::Statement statement(wuow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, group.id);
            statement.bindNoCopy(bindIndex++, group.name);
        }

        statement.exec();
        return {};
    } catch (const SQLite::Exception& ex) {
        return std::unexpected(mapSqliteException(ex));
    }
}

PersistenceResult<void> GroupRepository::remove(WriteUnitOfWork& wuow, int64_t id) {
    constexpr const char* const sql = {
        "DELETE FROM groups "
        "WHERE id = ?;"};

    try {
        SQLite::Statement statement(wuow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, id);
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

PersistenceResult<Group> GroupRepository::getById(UnitOfWork& uow, int64_t id) {
    constexpr const char* const sql = {
        "SELECT id, name "
        "FROM groups "
        "WHERE id = ?;"};

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

PersistenceResult<Group> GroupRepository::getByName(UnitOfWork& uow, const std::string& name) {
    constexpr const char* const sql = {
        "SELECT id, name "
        "FROM groups "
        "WHERE name = ?;"};

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

PersistenceResult<std::vector<Group>> GroupRepository::getAll(UnitOfWork& uow) {
    constexpr const char* const sql = {
        "SELECT id, name "
        "FROM groups;"};

    try {
        SQLite::Statement statement(uow.connection(), sql);

        std::vector<Group> groups;

        while (statement.executeStep()) {
            groups.emplace_back(readFromStatement(statement));
        }

        return groups;
    } catch (const SQLite::Exception& ex) {
        return std::unexpected(mapSqliteException(ex));
    }
}

}  // namespace infrastructure::repositories
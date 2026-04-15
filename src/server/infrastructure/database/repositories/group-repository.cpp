#include "group-repository.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline Group readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return Group{.id = stmt.getColumn(readIndex++),
                     .name = stmt.getColumn(readIndex++)};
    }

    void GroupRepository::create(WriteUnitOfWork& wuov, Group group) {
        constexpr const char* const sql = {
            "INSERT INTO groups "
                "(id, name) "
            "VALUES (?, ?);"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, group.id);
            statement.bindNoCopy(bindIndex++, group.name);
        }

        statement.executeStep();
    }

    void GroupRepository::remove(WriteUnitOfWork& wuov, int64_t id) {
        constexpr const char* const sql = {
            "DELETE FROM groups "
            "WHERE id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);

        {
            int bindIndex = 1;
            statement.bind(bindIndex++, id);
        }

        statement.executeStep();
    }

    Group GroupRepository::getById(UnitOfWork& uow, int64_t id) {
        constexpr const char* const sql = {
            "SELECT id, name "
            "FROM groups "
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

    Group GroupRepository::getByName(UnitOfWork& uow, const std::string& name) {
        constexpr const char* const sql = {
            "SELECT id, name "
            "FROM groups "
            "WHERE name = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        {
            int bindIndex = 1;
            statement.bindNoCopy(bindIndex++, name);
        }

        statement.executeStep();

        return readFromStatement(statement);
    }

    std::vector<Group> GroupRepository::getAll(UnitOfWork& uow) {
        constexpr const char* const sql = {
            "SELECT id, name "
            "FROM groups;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        std::vector<Group> groups;

        while (statement.executeStep()) {
            groups.emplace_back(readFromStatement(statement));
        }

        return groups;
    }

}
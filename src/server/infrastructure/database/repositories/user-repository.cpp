#include "user-repository.h"

#include <cstdint>

namespace infrastructure::db::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline User readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return User{.id = stmt.getColumn(readIndex++),
                    .login = stmt.getColumn(readIndex++),
                    .passwordHash = stmt.getColumn(readIndex++)};
    }

    void UserRepository::create(WriteUnitOfWork& wuov, User user) {
        constexpr const char* const sql = {
            "INSERT INTO users "
                "(id, login, password_hash) "
            "VALUES (?, ?, ?);"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, user.id);
            statement.bindNoCopy(bindIndex++, user.login);
            statement.bind(bindIndex++, user.passwordHash);
        }

        statement.executeStep();
    }

    void UserRepository::update(WriteUnitOfWork& wuov, User user) {
        constexpr const char* const sql = {
            "UPDATE users "
            "SET login = ?, "
                "password_hash = ? "
            "WHERE id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bindNoCopy(bindIndex++, user.login);
            statement.bind(bindIndex++, user.passwordHash);
            statement.bind(bindIndex++, user.id);
        }

        statement.executeStep();
    }

    void UserRepository::remove(WriteUnitOfWork& wuov, int64_t id) {
        constexpr const char* const sql = {
            "DELETE FROM users "
            "WHERE id = ?;"
        };

        SQLite::Statement statement(wuov.connection(), sql);
        {
            int bindIndex = 1;
            statement.bind(bindIndex++, id);
        }

        statement.executeStep();
    }

    User UserRepository::getById(UnitOfWork& uow, int64_t id) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users "
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

    User UserRepository::getByLogin(UnitOfWork& uow, const std::string& login) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users "
            "WHERE login = ?;"
        };

        SQLite::Statement statement(uow.connection(), sql);
        {
            int bindIndex = 1;
            statement.bindNoCopy(bindIndex++, login);
        }

        statement.executeStep();

        return readFromStatement(statement);
    }

    std::vector<User> UserRepository::getAll(UnitOfWork& uow) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users;"
        };

        SQLite::Statement statement(uow.connection(), sql);

        std::vector<User> users;
        while (statement.executeStep()) {
            users.emplace_back(readFromStatement(statement));
        }

        return users;
    }

}
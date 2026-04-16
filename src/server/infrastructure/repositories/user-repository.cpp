#include "user-repository.h"

namespace infrastructure::repositories {

    using namespace infrastructure::db::sqlite;
    using namespace domain::models;

    inline User readFromStatement(const SQLite::Statement& stmt) {
        int readIndex = 0;
        return User{
            .id = stmt.getColumn(readIndex++),
            .login = stmt.getColumn(readIndex++),
            .passwordHash = stmt.getColumn(readIndex++)
        };
    }

    PersistenceResult<void> UserRepository::create(WriteUnitOfWork& wuov, User user) {
        constexpr const char* const sql = {
            "INSERT INTO users "
                "(id, login, password_hash) "
            "VALUES (?, ?, ?);"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bind(bindIndex++, user.id);
                statement.bindNoCopy(bindIndex++, user.login);
                statement.bindNoCopy(bindIndex++, user.passwordHash);
            }

            statement.exec();
            return {};
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<void> UserRepository::update(WriteUnitOfWork& wuov, User user) {
        constexpr const char* const sql = {
            "UPDATE users "
            "SET login = ?, "
                "password_hash = ? "
            "WHERE id = ?;"
        };

        try {
            SQLite::Statement statement(wuov.connection(), sql);
            {
                int bindIndex = 1;
                statement.bindNoCopy(bindIndex++, user.login);
                statement.bindNoCopy(bindIndex++, user.passwordHash);
                statement.bind(bindIndex++, user.id);
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

    PersistenceResult<void> UserRepository::remove(WriteUnitOfWork& wuov, int64_t id) {
        constexpr const char* const sql = {
            "DELETE FROM users "
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

    PersistenceResult<User> UserRepository::getById(UnitOfWork& uow, int64_t id) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users "
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

    PersistenceResult<User> UserRepository::getByLogin(UnitOfWork& uow, const std::string& login) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users "
            "WHERE login = ?;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);
            {
                int bindIndex = 1;
                statement.bindNoCopy(bindIndex++, login);
            }

            if (!statement.executeStep()) {
                return std::unexpected(PersistenceError::NotFound);
            }

            return readFromStatement(statement);
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

    PersistenceResult<std::vector<User>> UserRepository::getAll(UnitOfWork& uow) {
        constexpr const char* const sql = {
            "SELECT id, login, password_hash "
            "FROM users;"
        };

        try {
            SQLite::Statement statement(uow.connection(), sql);

            std::vector<User> users;
            while (statement.executeStep()) {
                users.emplace_back(readFromStatement(statement));
            }

            return users;
        } catch (const SQLite::Exception& ex) {
            return std::unexpected(mapSqliteException(ex));
        }
    }

}
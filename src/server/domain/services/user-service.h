#pragma once

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/repositories/user-repository.h"

#include <string_view>
#include <cstdint>

namespace domain::services {

    using namespace infrastructure::db;
    using namespace domain::models;

    class UserService {

            sqlite::SqliteDatabase& m_database;

            repositories::UserRepository& m_userRepo;

        public:

            int64_t login(const std::string& login, const std::string& rawPassword) {
                auto rouw = m_database.createReadUnitOfWork();

                auto user = m_userRepo.getByLogin(rouw, login);

                if (user.passwordHash == hashPassword(rawPassword)) {
                    return user.id;
                }

                return {};
            }

            int64_t addUser(const std::string& login, const std::string& rawPassword) {
                auto wouw = m_database.createWriteUnitOfWork();
                int64_t newId = (int64_t)std::hash<std::string_view>{}(login);
                m_userRepo.create(wouw, User{.id = newId,
                                             .login = login,
                                             .passwordHash = hashPassword(rawPassword)});
                wouw.commit();
                return newId;
            }

        private:

            std::string hashPassword(std::string_view rawPassword);
    };

}
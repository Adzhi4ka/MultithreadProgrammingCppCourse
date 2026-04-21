#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/user-repository.h"

#include <cstdint>

namespace domain::services {

    class UserService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using UserRepository = infrastructure::repositories::UserRepository;

            SqliteDatabase& m_database;
            UserRepository& m_userRepo;

        public:

            UserService(SqliteDatabase& database, UserRepository& userRepo) noexcept;

            ServiceResult<int64_t> login(const std::string& login, const std::string& rawPassword);

            ServiceResult<int64_t> addUser(std::string login, std::string rawPassword);

    };

}
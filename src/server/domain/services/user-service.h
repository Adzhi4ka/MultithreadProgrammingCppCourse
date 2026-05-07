#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-group-repository.h"
#include "infrastructure/repositories/user-repository.h"

#include <cstdint>
#include <string>

#include "domain/models/user.h"

namespace domain::services {

    class UserService {

            using SqliteDatabase = infrastructure::db::sqlite::SqliteDatabase;
            using UserRepository = infrastructure::repositories::UserRepository;
            using GroupRepository = infrastructure::repositories::GroupRepository;
            using UserGroupRepository = infrastructure::repositories::UserGroupRepository;

            using User = domain::models::User;

            SqliteDatabase& m_database;
            UserRepository& m_userRepo;
            GroupRepository& m_groupRepo;
            UserGroupRepository& m_userGroupRepo;

        public:

            UserService(SqliteDatabase& database,
                        UserRepository& userRepo,
                        GroupRepository& groupRepo,
                        UserGroupRepository& userGroupRepo) noexcept;

            ServiceResult<int64_t> login(const std::string& login, const std::string& rawPassword);

            ServiceResult<int64_t> addUser(std::string login, std::string rawPassword);

            ServiceResult<User> getById(int64_t userId);
            ServiceResult<User> getByLogin(const std::string& login);

    };

}

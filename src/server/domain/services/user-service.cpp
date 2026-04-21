#include "user-service.h"

#include "infrastructure/id-generator/id-generator.h"
#include "infrastructure/security/password-hasher.h"

namespace {
    using namespace domain::models;
}

namespace domain::services {

    UserService::UserService(SqliteDatabase& database,
                             UserRepository& userRepo) noexcept
        : m_database(database),
          m_userRepo(userRepo) {}

    ServiceResult<int64_t> UserService::login(const std::string& login, const std::string& rawPassword) {
        auto ruow = m_database.createReadUnitOfWork();

        auto userResult = m_userRepo.getByLogin(ruow, login);
        if (!userResult) {
            return std::unexpected(mapPersistenceError(userResult.error()));
        }

        if (infrastructure::security::verifyPassword(rawPassword, userResult->passwordHash)) {
            return userResult->id;
        }

        return std::unexpected(ServiceError::Forbidden);
    }

    ServiceResult<int64_t> UserService::addUser(std::string login, std::string rawPassword) {
        auto wuow = m_database.createWriteUnitOfWork();

        int64_t newId = infrastructure::id_generator::generateId();

        auto createResult = m_userRepo.create(wuow, User{.id = newId,
                                                         .login = std::move(login),
                                                         .passwordHash = infrastructure::security::hashPassword(rawPassword)});

        if (!createResult) {
            return std::unexpected(mapPersistenceError(createResult.error()));
        }

        wuow.commit();

        return newId;
    }

}
#include "infrastructure/repositories/persistence-result.h"

#include <cstdint>
#include <expected>

namespace domain::services {

    enum class ServiceError : uint8_t {
        NotFound,
        Conflict,
        Forbidden,
        InternalError
    };

    template<typename T>
    using ServiceResult = std::expected<T, ServiceError>;

    using namespace infrastructure::repositories;

    inline ServiceError mapPersistenceError(PersistenceError err) noexcept {
        switch (err) {
            case PersistenceError::NotFound:
                return ServiceError::NotFound;
            case PersistenceError::Conflict:
                return ServiceError::Conflict;
            case PersistenceError::InternalError:
                return ServiceError::InternalError;
        }

        return ServiceError::InternalError;
    }

}
#pragma once

#include <cstdint>
#include <expected>

namespace infrastructure::db::repositories {

    enum class RepositoryError : uint8_t {
        NotFound,
        InternalError
    };

    template<typename T>
    using RepositoryOpResult = std::expected<T, RepositoryError>;

}
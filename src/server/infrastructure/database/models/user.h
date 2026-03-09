#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct User {
        int64_t id;
        std::string login;
        std::string passwordHash;
    };

}
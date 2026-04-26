#pragma once

#include <cstdint>
#include <string>

namespace domain::models {

    struct User {
        int64_t id;
        std::string login;
        std::string passwordHash;
    };

}
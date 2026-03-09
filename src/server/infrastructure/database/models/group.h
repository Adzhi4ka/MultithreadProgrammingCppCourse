#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct Group {
        int64_t id;
        std::string name;
    };

}
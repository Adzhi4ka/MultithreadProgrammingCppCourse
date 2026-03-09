#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct File {
        int64_t id;
        std::string name;
        int64_t version;
        int64_t created_at;
        int64_t modified_at;
        int64_t modified_by;
    };

}
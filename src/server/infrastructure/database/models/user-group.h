#pragma once

#include <cstdint>

namespace infrastructure::db::models {

    struct UserGroup {
        int64_t userId;
        int64_t groupId;
    };

}
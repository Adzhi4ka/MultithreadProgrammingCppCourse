#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct FileLock {
        int64_t fileId;
        int64_t userId;
        int64_t leaseUntil;
    };

}
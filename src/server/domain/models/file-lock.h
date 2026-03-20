#pragma once

#include <cstdint>

namespace domain::models {

    struct FileLock {
        int64_t fileId;
        int64_t userId;
        int64_t leaseUntil;
    };

}
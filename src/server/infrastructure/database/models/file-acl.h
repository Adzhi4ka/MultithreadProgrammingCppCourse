#pragma once

#include <cstdint>

namespace infrastructure::db::models {

    struct FileAcl {
        int64_t fileId;
        int64_t groupId;
        int8_t aclLevel;
    };

}
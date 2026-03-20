#pragma once

#include <cstdint>

namespace domain::models {

    enum class AclLevel : int8_t {
        READ_ONLY = 0,
        READ_WRITE = 1,
    };

    struct FileAcl {
        int64_t fileId;
        int64_t groupId;
        AclLevel aclLevel;
    };

}
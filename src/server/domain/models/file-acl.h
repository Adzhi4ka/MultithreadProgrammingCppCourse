#pragma once

#include <cstdint>

namespace domain::models {

    enum class AclLevel : int8_t {
        NO_PROPERTY = 0,
        READ_ONLY = 1,
        READ_WRITE = 2,
    };

    struct FileAcl {
        int64_t fileId;
        int64_t groupId;
        AclLevel aclLevel;
    };

}
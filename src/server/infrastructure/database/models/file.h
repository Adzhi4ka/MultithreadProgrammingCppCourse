#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct File {
        int64_t id;
        std::string fullLogicalName;
        int64_t currentVersionId;
        uint32_t maxVersionCount;
        int64_t createdAt;
        int64_t createdBy;
    };

}
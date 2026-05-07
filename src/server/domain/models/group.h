#pragma once

#include <cstdint>
#include <string>

namespace domain::models {

struct Group {
        int64_t id;
        std::string name;
};

}  // namespace domain::models
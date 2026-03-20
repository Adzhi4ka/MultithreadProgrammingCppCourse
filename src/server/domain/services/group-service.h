#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace domain::services {

    class GroupService {



        public:

            int64_t createGroup(std::string_view groupName);

            void addUserToGroup(int64_t userId, int64_t groupId);

            void removeUserFromGroup(int64_t userId, int64_t groupId);

            std::vector<int64_t> getUserGroups(int64_t userId);

    };

}
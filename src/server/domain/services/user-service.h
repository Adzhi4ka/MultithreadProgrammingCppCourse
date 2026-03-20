#pragma once

#include <string_view>
#include <cstdint>

namespace domain::services {

    class UserService {


        public:

            int64_t login(std::string_view login, std::string_view rawPassword);

            int64_t addUser(std::string_view login, std::string_view rawPassword);

        private:

            std::string hashPassword(std::string_view rawPassword);
    };

}
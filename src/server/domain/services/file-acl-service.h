#pragma once

#include <cstdint>

namespace domain::services {

    class FileAclService {


        
        public:

            AclLevel getGroupAclLevel(uint64_t groupId);

            AclLevel getUserAclLevel(uint64_t userId);
            
            void createGroupAclLevel(uint64_t fileId, uint64_t groupId, AclLevel aclLevel);

            void removeGroupAclLevel(uint64_t fileId, uint64_t groupId);

    };

};
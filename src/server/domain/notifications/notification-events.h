#pragma once

#include "infrastructure/event/event-bus.h"

#include <cstdint>
#include <string>

namespace domain::notifications {

    struct FileCreatedEvent {
        int64_t fileId;
        int64_t createdByUserId;
        int64_t currentVersionId;
        std::string logicalName;
    };

    struct FileLockedEvent {
        int64_t fileId;
        int64_t lockedByUserId;
        int64_t leaseUntil;
        int64_t lockToken;
    };

    struct FileUnlockedEvent {
        int64_t fileId;
        int64_t lockToken;
    };

    struct GroupAssignedEvent {
        int64_t userId;
        int64_t groupId;
        int64_t assignedByUserId;
        bool refreshFiles {true};
    };

    using NotificationEventBus = events::EventBus<FileCreatedEvent,
                                                  FileLockedEvent,
                                                  GroupAssignedEvent,
                                                  FileUnlockedEvent>;

}

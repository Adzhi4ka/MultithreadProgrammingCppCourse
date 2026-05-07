#include "sse-notification-bridge.h"

#include "json-helpers.h"

#include <boost/json.hpp>

namespace presentation::http {

    namespace json = boost::json;

    SseNotificationBridge::SseNotificationBridge(domain::notifications::NotificationEventBus& eventBus,
                                                 infrastructure::http::ActiveSessionRegistry& registry)
        : m_registry(registry) {
        m_fileCreatedConnection = eventBus.subscribe<domain::notifications::FileCreatedEvent>(
            [this](const domain::notifications::FileCreatedEvent& event) {
                onFileCreated(event);
            }
        );

        m_fileLockedConnection = eventBus.subscribe<domain::notifications::FileLockedEvent>(
            [this](const domain::notifications::FileLockedEvent& event) {
                onFileLocked(event);
            }
        );

        m_fileUnlockedConnection = eventBus.subscribe<domain::notifications::FileUnlockedEvent>(
            [this](const domain::notifications::FileUnlockedEvent& event) {
                onFileUnlocked(event);
            }
        );

        m_groupAssignedConnection = eventBus.subscribe<domain::notifications::GroupAssignedEvent>(
            [this](const domain::notifications::GroupAssignedEvent& event) {
                onGroupAssigned(event);
            }
        );
    }

    void SseNotificationBridge::onFileCreated(const domain::notifications::FileCreatedEvent& event) {
        json::object payload{
            {"type", "file_created"},
            {"fileId", event.fileId},
            {"logicalName", event.logicalName},
            {"currentVersionId", event.currentVersionId},
            {"createdByUserId", event.createdByUserId}
        };

        m_registry.publishToUser(event.createdByUserId,
                                 "file_created",
                                 serializeJson(payload));
    }

    void SseNotificationBridge::onFileLocked(const domain::notifications::FileLockedEvent& event) {
        json::object payload{
            {"type", "file_locked"},
            {"fileId", event.fileId},
            {"lockedByUserId", event.lockedByUserId},
            {"leaseUntil", event.leaseUntil},
            {"lockToken", event.lockToken}
        };

        m_registry.publishToAll("file_locked", serializeJson(payload));
    }

    void SseNotificationBridge::onFileUnlocked(const domain::notifications::FileUnlockedEvent& event) {
        json::object payload{
            {"type", "file_unlocked"},
            {"fileId", event.fileId},
            {"lockToken", event.lockToken}
        };

        m_registry.publishToAll("file_unlocked", serializeJson(payload));
    }

    void SseNotificationBridge::onGroupAssigned(const domain::notifications::GroupAssignedEvent& event) {
        json::object payload{
            {"type", "group_assigned"},
            {"userId", event.userId},
            {"groupId", event.groupId},
            {"assignedByUserId", event.assignedByUserId},
            {"refreshFiles", event.refreshFiles}
        };

        m_registry.publishToUser(event.userId,
                                 "group_assigned",
                                 serializeJson(payload));
    }

}

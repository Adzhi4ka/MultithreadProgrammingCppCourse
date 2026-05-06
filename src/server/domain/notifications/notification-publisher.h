#pragma once

#include "domain/notifications/notification-events.h"

#include <cstdint>
#include <string>
#include <utility>

namespace domain::notifications {

    class NotificationPublisher {

            NotificationEventBus& m_eventBus;

        public:

            explicit NotificationPublisher(NotificationEventBus& eventBus) noexcept
                : m_eventBus(eventBus) {}

            void fileCreated(int64_t fileId,
                             int64_t createdByUserId,
                             int64_t currentVersionId,
                             std::string logicalName) {
                m_eventBus.post(FileCreatedEvent{
                    .fileId = fileId,
                    .createdByUserId = createdByUserId,
                    .currentVersionId = currentVersionId,
                    .logicalName = std::move(logicalName)
                });
            }

            void fileLocked(int64_t fileId,
                            int64_t lockedByUserId,
                            int64_t leaseUntil,
                            int64_t lockToken) {
                m_eventBus.post(FileLockedEvent{
                    .fileId = fileId,
                    .lockedByUserId = lockedByUserId,
                    .leaseUntil = leaseUntil,
                    .lockToken = lockToken
                });
            }

            void fileUnlocked(int64_t fileId,
                              int64_t lockToken) {
                m_eventBus.post(FileUnlockedEvent{
                    .fileId = fileId,
                    .lockToken = lockToken
                });
            }

            void groupAssigned(int64_t userId,
                               int64_t groupId,
                               int64_t assignedByUserId) {
                m_eventBus.post(GroupAssignedEvent{
                    .userId = userId,
                    .groupId = groupId,
                    .assignedByUserId = assignedByUserId,
                    .refreshFiles = true
                });
            }

    };

}

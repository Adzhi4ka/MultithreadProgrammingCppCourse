#pragma once

#include "domain/notifications/notification-events.h"
#include "infrastructure/http/active-session-registry.h"

#include <boost/signals2/connection.hpp>

namespace presentation::http {

    class SseNotificationBridge {

            infrastructure::http::ActiveSessionRegistry& m_registry;

            boost::signals2::scoped_connection m_fileCreatedConnection;
            boost::signals2::scoped_connection m_fileLockedConnection;
            boost::signals2::scoped_connection m_groupAssignedConnection;

        public:

            SseNotificationBridge(domain::notifications::NotificationEventBus& eventBus,
                                  infrastructure::http::ActiveSessionRegistry& registry);

        private:

            void onFileCreated(const domain::notifications::FileCreatedEvent& event);
            void onFileLocked(const domain::notifications::FileLockedEvent& event);
            void onGroupAssigned(const domain::notifications::GroupAssignedEvent& event);

    };

}

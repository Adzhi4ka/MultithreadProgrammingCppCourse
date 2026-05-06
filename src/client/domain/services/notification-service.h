#pragma once

#include "domain/services/remote-service-base.h"

#include <functional>

namespace client::domain::services {

    class NotificationService : public RemoteServiceBase {

            using NotificationEventResult = ApiResult<domain::models::NotificationEvent>;

        public:

            NotificationService(application::NetworkWorker& networkWorker,
                                QObject& internalContext,
                                QObject& uiContext) noexcept;

            void start(std::function<void(NotificationEventResult)> callback);
            void stop();

    };

}

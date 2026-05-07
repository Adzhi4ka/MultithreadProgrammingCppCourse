#pragma once

#include <functional>

#include "domain/services/remote-service-base.h"

namespace client::domain::services {

class NotificationService : public RemoteServiceBase {

        using NotificationEventResult = ApiResult<domain::models::NotificationEvent>;

    public:

        NotificationService(application::NetworkWorker& networkWorker, QObject& internalContext,
                            QObject& uiContext) noexcept;

        void start(std::function<void(NotificationEventResult)> callback);
        void stop();
};

}  // namespace client::domain::services

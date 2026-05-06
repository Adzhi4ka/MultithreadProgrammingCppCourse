#include "notification-service.h"

#include <QPointer>

#include <memory>
#include <utility>

namespace client::domain::services {

    NotificationService::NotificationService(application::NetworkWorker& networkWorker,
                                             QObject& internalContext,
                                             QObject& uiContext) noexcept
        : RemoteServiceBase(networkWorker, internalContext, uiContext) {}

    void NotificationService::start(std::function<void(NotificationEventResult)> callback) {
        auto cb = std::make_shared<std::function<void(NotificationEventResult)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};

        m_networkWorker.run([internalContext, uiContext, cb](RemoteApiGateway& gateway) mutable {
            gateway.startNotifications([internalContext, uiContext, cb](NotificationEventResult result) mutable {
                ::client::application::postTask(internalContext,
                                      [uiContext, cb, result = std::move(result)]() mutable {
                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void NotificationService::stop() {
        m_networkWorker.run([](RemoteApiGateway& gateway) {
            gateway.stopNotifications();
        });
    }

}

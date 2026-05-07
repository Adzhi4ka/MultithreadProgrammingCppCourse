#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <utility>

#include "application/dispatch-helpers.h"
#include "application/network-worker.h"

namespace client::domain::services {

class RemoteServiceBase {

    protected:

        using RemoteApiGateway = infrastructure::api::RemoteApiGateway;

        application::NetworkWorker& m_networkWorker;
        QObject& m_internalContext;
        QObject& m_uiContext;

    public:

        RemoteServiceBase(application::NetworkWorker& networkWorker, QObject& internalContext,
                          QObject& uiContext) noexcept
            : m_networkWorker(networkWorker), m_internalContext(internalContext), m_uiContext(uiContext) {}

    protected:

        template <typename Callback, typename Result>
        void postUi(std::shared_ptr<Callback> callback, Result result) const {
            application::postResult(QPointer<QObject>{&m_uiContext}, std::move(callback), std::move(result));
        }

        template <typename Task>
        void postInternal(Task&& task) const {
            application::postTask(QPointer<QObject>{&m_internalContext}, std::forward<Task>(task));
        }

        template <typename Callback, typename Submit>
        void runSimple(Callback callback, Submit&& submit) {
            auto cb = std::make_shared<Callback>(std::move(callback));
            QPointer<QObject> internalContext{&m_internalContext};
            QPointer<QObject> uiContext{&m_uiContext};

            m_networkWorker.run([internalContext, uiContext, cb,
                                 submit = std::forward<Submit>(submit)](RemoteApiGateway& gateway) mutable {
                auto done = [internalContext, uiContext, cb](auto result) mutable {
                    application::postTask(internalContext, [uiContext, cb, result = std::move(result)]() mutable {
                        application::postResult(uiContext, cb, std::move(result));
                    });
                };

                submit(gateway, std::move(done));
            });
        }
};

}  // namespace client::domain::services

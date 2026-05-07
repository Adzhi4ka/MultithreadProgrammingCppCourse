#pragma once

#include "infrastructure/api/remote-api-gateway.h"

#include <QObject>
#include <QPointer>
#include <QUrl>

#include <memory>
#include <utility>

namespace client::application {

    class NetworkWorker : public QObject {

            Q_OBJECT
            QUrl m_baseUrl;
            std::unique_ptr<infrastructure::api::RemoteApiGateway> m_gateway;

        public:

            explicit NetworkWorker(QUrl baseUrl, QObject* parent = nullptr);
            ~NetworkWorker() override;

            void shutdown();

            NetworkWorker(const NetworkWorker&) = delete;
            NetworkWorker& operator=(const NetworkWorker&) = delete;

            template <typename Task>
            void run(Task&& task) {
                QPointer<NetworkWorker> guard{this};
                QMetaObject::invokeMethod(this,
                                        [guard, task = std::forward<Task>(task)]() mutable {
                                            if (!guard) {
                                                return;
                                            }

                                            guard->ensureGateway();
                                            task(*guard->m_gateway);
                                        },
                                        Qt::QueuedConnection);
            }

        private:

            void ensureGateway();

    };

}

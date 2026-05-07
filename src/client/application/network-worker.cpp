#include "network-worker.h"

#include <utility>

namespace client::application {

NetworkWorker::NetworkWorker(QUrl baseUrl, QObject* parent) : QObject(parent), m_baseUrl(std::move(baseUrl)) {}

NetworkWorker::~NetworkWorker() = default;

void NetworkWorker::shutdown() {
    if (m_gateway) {
        m_gateway->stopNotifications();
        m_gateway.reset();
    }
}

void NetworkWorker::ensureGateway() {
    if (!m_gateway) {
        m_gateway = std::make_unique<infrastructure::api::RemoteApiGateway>(m_baseUrl);
    }
}

}  // namespace client::application

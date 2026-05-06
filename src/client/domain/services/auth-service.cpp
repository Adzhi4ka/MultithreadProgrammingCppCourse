#include "auth-service.h"

#include <QPointer>

#include <memory>
#include <utility>

namespace client::domain::services {

    AuthService::AuthService(application::NetworkWorker& networkWorker,
                             QObject& internalContext,
                             QObject& uiContext,
                             infrastructure::repositories::SessionRepository& sessionRepository) noexcept
        : RemoteServiceBase(networkWorker, internalContext, uiContext),
          m_sessionRepository(sessionRepository) {}

    void AuthService::authenticate(QUrl baseUrl,
                                   QString login,
                                   QString password,
                                   bool registration,
                                   std::function<void(ApiResult<UserSession>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<UserSession>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* sessionRepository = &m_sessionRepository;

        m_networkWorker.run([baseUrl = std::move(baseUrl),
                             login = std::move(login),
                             password = std::move(password),
                             registration,
                             internalContext,
                             uiContext,
                             cb,
                             sessionRepository](infrastructure::api::RemoteApiGateway& gateway) mutable {
            gateway.setBaseUrl(std::move(baseUrl));
            gateway.clearBearerToken();

            auto* gatewayPtr = &gateway;
            auto done = [gatewayPtr, internalContext, uiContext, cb, sessionRepository](ApiResult<UserSession> result) mutable {
                if (result) {
                    gatewayPtr->setBearerToken(result->token);
                }

                ::client::application::postTask(internalContext,
                                      [uiContext, cb, sessionRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              sessionRepository->save(*result);
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            };

            if (registration) {
                gateway.authApi().registerUser(login, password, std::move(done));
            } else {
                gateway.authApi().login(login, password, std::move(done));
            }
        });
    }

    void AuthService::setSession(UserSession session) {
        m_sessionRepository.save(session);
        m_networkWorker.run([session = std::move(session)](infrastructure::api::RemoteApiGateway& gateway) mutable {
            gateway.setBearerToken(std::move(session.token));
        });
    }

    void AuthService::logout() {
        m_sessionRepository.clear();
        m_networkWorker.run([](infrastructure::api::RemoteApiGateway& gateway) {
            gateway.clearBearerToken();
        });
    }

}

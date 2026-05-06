#pragma once

#include "api-client.h"
#include "api-result.h"
#include "domain/models/file-version.h"

#include <QByteArray>
#include <QObject>

#include <functional>

namespace client::infrastructure::api {

    class FileContentApi : public QObject {

            Q_OBJECT
            ApiClient& m_apiClient;

        public:
            explicit FileContentApi(ApiClient& apiClient, QObject* parent = nullptr);

            void downloadCurrent(qint64 fileId, std::function<void(ApiResult<QByteArray>)> callback);
            void uploadCurrent(qint64 fileId,
                               const QByteArray& content,
                               std::function<void(ApiResult<domain::models::FileVersion>)> callback);
            void downloadVersion(qint64 versionId, std::function<void(ApiResult<QByteArray>)> callback);

        private:

            static ApiResult<domain::models::FileVersion> parseVersionResponse(const RawApiResponse& response);

    };

}

#pragma once

#include "api-client.h"
#include "api-result.h"
#include "domain/models/file-version.h"

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class FileVersionApi : public QObject {

            using FileVersion = domain::models::FileVersion;

            Q_OBJECT
            ApiClient& m_apiClient;

        public:

            explicit FileVersionApi(ApiClient& apiClient, QObject* parent = nullptr);

            void create(qint64 fileId,
                        const QString& logicalNameSnapshot,
                        std::function<void(ApiResult<FileVersion>)> callback);
            void getCurrent(qint64 fileId, std::function<void(ApiResult<FileVersion>)> callback);
            void getAll(qint64 fileId,
                        std::function<void(ApiResult<std::vector<FileVersion>>)> callback);

        private:

            static ApiResult<FileVersion> parseVersionResponse(const RawApiResponse& response);
            static ApiResult<std::vector<FileVersion>> parseVersionsResponse(const RawApiResponse& response);
    };

}

#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/file-version.h"

#include <QByteArray>
#include <QObject>

#include <functional>

namespace client::infrastructure::api {

    class FileContentApi final : public QObject {
        Q_OBJECT

    public:
        using BytesResult = ApiResult<QByteArray>;
        using BytesCallback = std::function<void(BytesResult)>;
        using VersionResult = ApiResult<domain::models::FileVersion>;
        using VersionCallback = std::function<void(VersionResult)>;

        explicit FileContentApi(ApiClient& apiClient, QObject* parent = nullptr);

        void downloadCurrent(qint64 fileId, BytesCallback callback);
        void uploadCurrent(qint64 fileId, const QByteArray& content, VersionCallback callback);
        void downloadVersion(qint64 versionId, BytesCallback callback);

    private:
        static VersionResult parseVersionResponse(const RawApiResponse& response);

    private:
        ApiClient& m_apiClient;
    };

}

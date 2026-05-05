#include "file-version-api.h"

#include "json-utils.h"

#include <QJsonObject>
#include <QUrlQuery>

namespace client::infrastructure::api {

    FileVersionApi::FileVersionApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void FileVersionApi::create(qint64 fileId, const QString& logicalNameSnapshot, VersionCallback callback) {
        QJsonObject body{
            {"fileId", fileId},
            {"logicalNameSnapshot", logicalNameSnapshot},
        };

        m_apiClient.postJson("/api/file-versions", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseVersionResponse(response));
        });
    }

    void FileVersionApi::getCurrent(qint64 fileId, VersionCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.get("/api/file-versions/current", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseVersionResponse(response));
        });
    }

    void FileVersionApi::getAll(qint64 fileId, VersionsCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.get("/api/file-versions", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseVersionsResponse(response));
        });
    }

    FileVersionApi::VersionResult FileVersionApi::parseVersionResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "file version operation failed"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseFileVersion(*object));
    }

    FileVersionApi::VersionsResult FileVersionApi::parseVersionsResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "failed to load file versions"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseFileVersionItems(*object));
    }

}

#include "file-content-api.h"

#include <QUrlQuery>

#include "json-utils.h"

namespace client::infrastructure::api {

FileContentApi::FileContentApi(ApiClient& apiClient, QObject* parent) : QObject(parent), m_apiClient(apiClient) {}

void FileContentApi::downloadCurrent(qint64 fileId, std::function<void(ApiResult<QByteArray>)> callback) {
    QUrlQuery query;
    query.addQueryItem("fileId", QString::number(fileId));

    m_apiClient.get("/api/files/content", query, [callback = std::move(callback)](RawApiResponse response) mutable {
        if (!response.isSuccessStatus()) {
            callback(apiFailure(makeHttpError(response, "failed to download file content")));
            return;
        }

        callback(apiSuccess(std::move(response.body)));
    });
}

void FileContentApi::uploadCurrent(qint64 fileId, const QByteArray& content,
                                   std::function<void(ApiResult<domain::models::FileVersion>)> callback) {
    QUrlQuery query;
    query.addQueryItem("fileId", QString::number(fileId));

    m_apiClient.putRaw("/api/files/content", query, content, "application/octet-stream",
                       [callback = std::move(callback)](RawApiResponse response) mutable {
                           callback(parseVersionResponse(response));
                       });
}

void FileContentApi::downloadVersion(qint64 versionId, std::function<void(ApiResult<QByteArray>)> callback) {
    QUrlQuery query;
    query.addQueryItem("versionId", QString::number(versionId));

    m_apiClient.get("/api/file-versions/content", query,
                    [callback = std::move(callback)](RawApiResponse response) mutable {
                        if (!response.isSuccessStatus()) {
                            callback(apiFailure(makeHttpError(response, "failed to download file version content")));
                            return;
                        }

                        callback(apiSuccess(std::move(response.body)));
                    });
}

ApiResult<domain::models::FileVersion> FileContentApi::parseVersionResponse(const RawApiResponse& response) {
    if (!response.isSuccessStatus()) {
        return apiFailure(makeHttpError(response, "failed to upload file content"));
    }

    QString parseError;
    const auto object = parseJsonObject(response.body, &parseError);
    if (!object) {
        return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
    }

    return apiSuccess(parseFileVersion(*object));
}

}  // namespace client::infrastructure::api

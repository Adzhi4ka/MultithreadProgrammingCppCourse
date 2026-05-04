#include "file-api.h"

#include "json-utils.h"

#include <QJsonObject>
#include <QUrlQuery>

namespace client::infrastructure::api {

    FileApi::FileApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void FileApi::getAll(FilesCallback callback) {
        m_apiClient.get("/api/files", {}, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to load files")));
                return;
            }

            QString parseError;
            const auto object = parseJsonObject(response.body, &parseError);
            if (!object) {
                callback(apiFailure(ApiError{response.httpStatus, "invalid_json", parseError}));
                return;
            }

            callback(apiSuccess(parseRemoteFileItems(*object)));
        });
    }

    void FileApi::getById(qint64 fileId, FileCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.get("/api/files/by-id", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseSingleFileResponse(response));
        });
    }

    void FileApi::getByLogicalName(const QString& logicalName, FileCallback callback) {
        QUrlQuery query;
        query.addQueryItem("logicalName", logicalName);

        m_apiClient.get("/api/files/by-name", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseSingleFileResponse(response));
        });
    }

    void FileApi::create(const QString& logicalName, quint32 maxVersionCount, FileCallback callback) {
        QJsonObject body{
            {"logicalName", logicalName},
            {"maxVersionCount", (qint64)maxVersionCount},
        };

        m_apiClient.postJson("/api/files", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseSingleFileResponse(response));
        });
    }

    void FileApi::rename(qint64 fileId, const QString& newLogicalName, FileCallback callback) {
        QJsonObject body{
            {"fileId", fileId},
            {"newLogicalName", newLogicalName},
        };

        m_apiClient.putJson("/api/files/rename", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseSingleFileResponse(response));
        });
    }

    void FileApi::remove(qint64 fileId, VoidCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.deleteRequest("/api/files", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to remove file")));
                return;
            }

            callback(apiSuccess());
        });
    }

    FileApi::FileResult FileApi::parseSingleFileResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "file operation failed"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseRemoteFile(*object));
    }

}

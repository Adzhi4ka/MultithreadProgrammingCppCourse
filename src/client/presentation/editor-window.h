#pragma once

#include "application/client-runtime.h"
#include "domain/models/file-version.h"

#include <QMainWindow>

#include <optional>

class QLabel;
class QPushButton;
class QTextEdit;
class QTimer;

namespace client::presentation {

    class EditorWindow : public QMainWindow {

            Q_OBJECT

            static constexpr qint64 LockDurationSec = 300;

            application::ClientRuntime& m_runtime;
            QString m_title;
            std::optional<qint64> m_fileId;
            std::optional<qint64> m_lockToken;

            QTextEdit* m_editor = nullptr;
            QLabel* m_statusLabel = nullptr;
            QPushButton* m_saveButton = nullptr;
            QPushButton* m_releaseButton = nullptr;
            QTimer* m_renewTimer = nullptr;
            bool m_releaseStarted = false;

        public:
            static EditorWindow* createReadOnly(application::ClientRuntime& runtime,
                                                QString title,
                                                QByteArray content,
                                                QWidget* parent = nullptr);
            static EditorWindow* createEditable(application::ClientRuntime& runtime,
                                                qint64 fileId,
                                                qint64 lockToken,
                                                QString logicalName,
                                                QByteArray content,
                                                QWidget* parent = nullptr);

        private slots:
            void save();
            void releaseLock();
            void renewLock();
            void handleCurrentUploaded(qint64 fileId, ApiResult<domain::models::FileVersion> result);
            void handleLockReleased(qint64 fileId, qint64 lockToken, ApiResult<void> result);
            void handleLockRenewed(qint64 fileId, qint64 lockToken, ApiResult<void> result);

        private:
            void closeEvent(QCloseEvent* event) override;

            EditorWindow(application::ClientRuntime& runtime,
                         QString title,
                         QByteArray content,
                         std::optional<qint64> fileId,
                         std::optional<qint64> lockToken,
                         QWidget* parent = nullptr);

            void buildUi(const QByteArray& content);
            void connectRuntimeSignals();
            void setBusy(bool busy);
            void showApiError(const QString& title, const ApiError& error);

    };

}

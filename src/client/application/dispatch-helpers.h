#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <memory>
#include <utility>

namespace client::application {

template <typename Task>
void postTask(QPointer<QObject> context, Task&& task) {
    if (!context) {
        return;
    }

    QMetaObject::invokeMethod(
        context.data(),
        [context, task = std::forward<Task>(task)]() mutable {
            if (!context) {
                return;
            }

            task();
        },
        Qt::QueuedConnection);
}

template <typename Task>
void postTask(QObject* context, Task&& task) {
    postTask(QPointer<QObject>{context}, std::forward<Task>(task));
}

template <typename Callback, typename Result>
void postResult(QPointer<QObject> context, std::shared_ptr<Callback> callback, Result result) {
    postTask(context, [callback = std::move(callback), result = std::move(result)]() mutable {
        if (!callback || !*callback) {
            return;
        }

        (*callback)(std::move(result));
    });
}

template <typename Callback, typename Result>
void postResult(QObject* context, std::shared_ptr<Callback> callback, Result result) {
    postResult(QPointer<QObject>{context}, std::move(callback), std::move(result));
}

}  // namespace client::application

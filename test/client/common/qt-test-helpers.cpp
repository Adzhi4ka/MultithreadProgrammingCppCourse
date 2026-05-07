#include "qt-test-helpers.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>

namespace tests::client {

bool waitUntil(const std::function<bool()>& condition, int timeoutMs) {
    QElapsedTimer timer;
    timer.start();

    while (!condition()) {
        if (timer.elapsed() >= timeoutMs) {
            return condition();
        }

        QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
        QThread::msleep(5);
    }

    return true;
}

}  // namespace tests::client

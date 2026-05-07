#pragma once

#include <functional>

namespace tests::client {

bool waitUntil(const std::function<bool()>& condition, int timeoutMs = 3000);

}

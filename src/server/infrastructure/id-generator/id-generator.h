#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace infrastructure::id_generator {

inline int64_t generateId() noexcept {

    using namespace std::chrono;

    static auto getTime = []() { return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count(); };

    static std::atomic<int64_t> lastValue{getTime()};

    int64_t currentTime = getTime();

    int64_t last = lastValue.load(std::memory_order_relaxed);

    while (true) {
        const int64_t candidate = currentTime > last ? currentTime : last + 1;

        if (lastValue.compare_exchange_weak(last, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {
            return candidate;
        }

        currentTime = getTime();
    }
}

}  // namespace infrastructure::id_generator
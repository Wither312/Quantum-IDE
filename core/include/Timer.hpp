#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>

namespace core {

    class Timer {
    public:
        Timer() noexcept;

        // Resets the timer to now
        void reset() noexcept;

        // Returns elapsed time in seconds as double
        [[nodiscard]] double elapsed() const noexcept;

        // Returns elapsed time in milliseconds as integral type
        template<std::integral Int = int64_t>
        [[nodiscard]] Int elapsedMillis() const noexcept {
            using namespace std::chrono;
            return duration_cast<milliseconds>(high_resolution_clock::now() - m_start).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point m_start;
    };

} // namespace core

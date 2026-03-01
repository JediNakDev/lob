#ifndef LOB_ENGINE_SPSC_QUEUE_HPP
#define LOB_ENGINE_SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <cstddef>

namespace lob::engine {

template <typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert(Capacity > 1, "Capacity must be > 1");

public:
    bool try_push(const T& value) noexcept {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t next = increment(tail);
        if (next == head_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[tail] = value;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    bool try_pop(T& out) noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        out = buffer_[head];
        head_.store(increment(head), std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

private:
    static constexpr std::size_t increment(std::size_t idx) noexcept {
        return (idx + 1) % Capacity;
    }

    alignas(128) std::atomic<std::size_t> head_{0};
    alignas(128) std::atomic<std::size_t> tail_{0};
    alignas(128) std::array<T, Capacity> buffer_{};
};

}  // namespace lob::engine

#endif

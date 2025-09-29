
#pragma once
#include <atomic>
#include <cstddef>
#include <vector>
#include <cassert>
#include <utility>
#include <expected> 
#include <concepts>

template <typename T>
class Ringbuffer {
public:
    enum class Error { Full, Empty };

    explicit Ringbuffer(std::size_t capacity_pow2) {
        assert(capacity_pow2 && (capacity_pow2 & (capacity_pow2 - 1)) == 0 &&
               "capacity must be a power of two");
        cap = capacity_pow2;
        mask = cap - 1;
        buffer.resize(cap);
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    Ringbuffer(const Ringbuffer&) = delete;
    Ringbuffer& operator=(const Ringbuffer&) = delete;

    std::expected<void, Error> push(const T& value) {
        const auto t = tail.load(std::memory_order_relaxed);
        const auto next_t = (t + 1) & mask;

        if (next_t == head.load(std::memory_order_acquire)) {
            return std::unexpected(Error::Full);
        }

        buffer[t] = value;
        tail.store(next_t, std::memory_order_release);
        return {};
    }

    std::expected<void, Error> push(T&& value) {
        const auto t = tail.load(std::memory_order_relaxed);
        const auto next_t = (t + 1) & mask;

        if (next_t == head.load(std::memory_order_acquire)) {
            return std::unexpected(Error::Full);
        }

        buffer[t] = std::move(value);
        tail.store(next_t, std::memory_order_release);
        return {};
    }

    std::expected<T, Error> pop() {
        const auto h = head.load(std::memory_order_relaxed);

        if (h == tail.load(std::memory_order_acquire)) {
            return std::unexpected(Error::Empty);
        }

        T value = std::move(buffer[h]);
        head.store((h + 1) & mask, std::memory_order_release);
        return value;
    }

    bool empty() const noexcept {
        return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
    }

    std::size_t capacity() const noexcept { return cap - 1; }

private:
    std::vector<T> buffer;
    std::size_t cap;
    std::size_t mask;
    alignas(64) std::atomic<std::size_t> head; // consumer index
    alignas(64) std::atomic<std::size_t> tail; // producer index
};


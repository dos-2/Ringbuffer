# Ringbuffer
cache aligned lock free ring buffer c++ 23

# Lock-Free Ring Buffer

A simple, header-only **cache aligned lock-free ring buffer** implementation in C++23.

## Features

- **Lock-free** single-producer/single-consumer design
- Provides a `std::expected`-based API for error handling (`Full`, `Empty`)
- Thread-safe push/pop for one producer and one consumer
- Lightweight and cache-friendly
- Includes GoogleTest unit tests

## Requirements

- **C++23** (for `std::expected`)
- **CMake 3.20+**
- [GoogleTest](https://github.com/google/googletest) (automatically fetched via CMake if not installed)

## Build Instructions

Clone and build:

```
git clone https://github.com/dos-2/ringbuffer.git
cd ringbuffer
mkdir build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_CXX_STANDARD=23
make
```

## Example Usage

```
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include "Ringbuffer.hpp"

int main() {
    Ringbuffer<int> q(16);
    std::mutex cout_mutex;

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 20; ++i) {
            auto res = q.push(i);
            while (!res.has_value()) {
                if (res.error() == Ringbuffer<int>::Error::Full) {
                    res = q.push(i); // retry if full
                } else {
                    throw std::runtime_error("Unexpected push error");
                }
            }

            {
                std::scoped_lock lock(cout_mutex);
                std::cout << "Produced " << i << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // Consumer thread
    std::thread consumer([&]() {
        for (int i = 0; i < 20; ++i) {
            auto val = q.pop();
            while (!val.has_value()) {
                if (val.error() == Ringbuffer<int>::Error::Empty) {
                    val = q.pop(); // retry if empty
                } else {
                    throw std::runtime_error("Unexpected pop error");
                }
            }

            {
                std::scoped_lock lock(cout_mutex);
                std::cout << "Consumed " << val.value() << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(7));
        }
    });

    producer.join();
    consumer.join();

    return 0;
}
```

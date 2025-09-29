
#include <gtest/gtest.h>
#include "Ringbuffer.hpp"
#include <thread>
#include <vector>

TEST(RingbufferTest, SingleThreadPushPop) {
    Ringbuffer<int> q(8);

    EXPECT_TRUE(q.empty());

    for (int i = 0; i < 7; ++i) {
        auto res = q.push(i);
        EXPECT_TRUE(res.has_value());
    }

    auto res_full = q.push(100);
    EXPECT_FALSE(res_full.has_value());
    EXPECT_EQ(res_full.error(), Ringbuffer<int>::Error::Full);

    for (int i = 0; i < 7; ++i) {
        auto val = q.pop();
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i);
    }

    EXPECT_TRUE(q.empty());
    auto val_empty = q.pop();
    EXPECT_FALSE(val_empty.has_value());
    EXPECT_EQ(val_empty.error(), Ringbuffer<int>::Error::Empty);
}

TEST(RingbufferTest, MultiThreadPushPop) {
    Ringbuffer<int> q(16);
    std::vector<int> consumed;
    consumed.reserve(10'000);

    std::thread producer([&]() {
        for (int i = 0; i < 10'000; ++i) {
            while (true) {
                auto res = q.push(i);
                if (res.has_value()) break; 
            }
        }
    });

    std::thread consumer([&]() {
        for (int i = 0; i < 10'000; ++i) {
            while (true) {
                auto val = q.pop();
                if (val.has_value()) {
                    consumed.push_back(val.value());
                    break;
                }
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(consumed.size(), 10'000);
    for (int i = 0; i < 10'000; ++i) {
        EXPECT_EQ(consumed[i], i);
    }
}

TEST(RingbufferTest, FullQueue) {
    Ringbuffer<int> q(4); 

    EXPECT_TRUE(q.push(1).has_value());
    EXPECT_TRUE(q.push(2).has_value());
    EXPECT_TRUE(q.push(3).has_value());

    auto res_full = q.push(4);
    EXPECT_FALSE(res_full.has_value());
    EXPECT_EQ(res_full.error(), Ringbuffer<int>::Error::Full);
}

TEST(RingbufferTest, EmptyQueue) {
    Ringbuffer<int> q(4);

    auto val_empty = q.pop();
    EXPECT_FALSE(val_empty.has_value());
    EXPECT_EQ(val_empty.error(), Ringbuffer<int>::Error::Empty);
}

TEST(RingbufferTest, WrapAround) {
    Ringbuffer<int> q(4); 

    EXPECT_TRUE(q.push(10).has_value());
    EXPECT_TRUE(q.push(20).has_value());
    EXPECT_TRUE(q.push(30).has_value());

    auto v1 = q.pop(); EXPECT_EQ(v1.value(), 10);
    auto v2 = q.pop(); EXPECT_EQ(v2.value(), 20);

    EXPECT_TRUE(q.push(40).has_value());
    EXPECT_TRUE(q.push(50).has_value());

    EXPECT_EQ(q.pop().value(), 30);
    EXPECT_EQ(q.pop().value(), 40);
    EXPECT_EQ(q.pop().value(), 50);
    auto val_empty = q.pop();
    EXPECT_FALSE(val_empty.has_value());
    EXPECT_EQ(val_empty.error(), Ringbuffer<int>::Error::Empty);
}


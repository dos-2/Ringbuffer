include(FetchContent)

# Download GoogleTest if not already available
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# Make it available
FetchContent_MakeAvailable(googletest)
# Add test executable
add_executable(Ringbuffer_test Ringbuffer.t.cpp)

# Link the header-only library
target_link_libraries(Ringbuffer_test PRIVATE Ringbuffer GTest::gtest_main)

enable_testing()

# Optional: automatically discover tests if using GoogleTest
include(GoogleTest)
gtest_discover_tests(Ringbuffer_test)

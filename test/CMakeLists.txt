cmake_minimum_required(VERSION 3.14)

project(test)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${PROJECT_NAME} main.cpp test.cpp)

target_link_libraries(${PROJECT_NAME}  GTest::gtest_main src)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
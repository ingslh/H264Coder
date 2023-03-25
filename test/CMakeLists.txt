cmake_minimum_required(VERSION 3.14)

project(test)

message(${CMAKE_CURRENT_LIST_DIR})
set(ASSERT_PATH  "${CMAKE_CURRENT_LIST_DIR}/../assets/" CACHE PATH "assets path")
configure_file(
	"${PROJECT_SOURCE_DIR}/test_config.h.in"
    "${PROJECT_SOURCE_DIR}/test_config.h"
)

include_directories(
    ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include
    ${PROJECT_SOURCE_DIR}/src
)

link_directories(${PROJECT_SOURCE_DIR}/third_party/googletest/build/lib)

add_executable(${PROJECT_NAME} test.cpp main.cpp)

target_link_libraries(${PROJECT_NAME} gtest src)




cmake_minimum_required(VERSION 3.14)
project(src)

aux_source_directory(. source)
add_library(${PROJECT_NAME} ${source})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
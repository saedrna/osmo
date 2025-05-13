cmake_minimum_required(VERSION 3.10)
project(Osmo)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(simpleble REQUIRED)

add_subdirectory(dji)

add_executable(Osmo main.cpp)
target_link_libraries(Osmo simpleble::simpleble dji)

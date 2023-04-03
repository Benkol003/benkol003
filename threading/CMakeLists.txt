cmake_minimum_required(VERSION 3.26)
project(test)

find_package(benchmark REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(test1 main1.cpp)

add_executable(test2 main2.cpp)

target_link_libraries(test1 benchmark::benchmark)

target_link_libraries(test2 benchmark::benchmark)

cmake_minimum_required(VERSION 3.16)
project(mini_dfs_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party)

add_executable(storage_node storage_node/main.cpp)
add_executable(metadata_service metadata_service/main.cpp)
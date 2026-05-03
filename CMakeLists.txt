cmake_minimum_required(VERSION 3.16)

project(mini_dfs_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(storage_node
    storage_node/main.cpp
)

add_executable(metadata_node
    metadata_node/main.cpp
)

add_executable(dfs
    client/dfs.cpp
)

add_executable(cache_node
    cache_node/main.cpp
    cache_node/cache.c
    cache_node/cache_stats.c
    cache_node/cache_analyzer.c
)

target_include_directories(storage_node PRIVATE
    third_party
)

target_include_directories(metadata_node PRIVATE
    third_party
)

target_include_directories(dfs PRIVATE
    third_party
)

target_include_directories(cache_node PRIVATE
    cache_node
)
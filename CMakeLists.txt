cmake_minimum_required(VERSION 3.16)
project(BitTorrentRefactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
        torrent_lib
        src/bencode.cpp
        src/torrent_meta.cpp
        src/tracker.cpp
)

target_include_directories(torrent_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
        bt_main src/main.cpp
        include/torrent/tracker.hpp
)
target_link_libraries(bt_main PRIVATE torrent_lib)

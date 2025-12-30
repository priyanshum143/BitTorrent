cmake_minimum_required(VERSION 3.16)
project(BitTorrentRefactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    torrent_lib
    src/bencode.cpp
    src/torrent_meta.cpp
    src/tracker.cpp
    src/peer.cpp
    src/peer_messages.cpp
    src/piece_downloader.cpp
    src/net_utils.cpp
    src/file_downloader.cpp
)

target_include_directories(torrent_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(torrent_lib
    PRIVATE
        CURL::libcurl
        OpenSSL::Crypto
)

add_executable(bt_main src/main.cpp)
target_link_libraries(bt_main PRIVATE torrent_lib)

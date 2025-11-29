#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace torrent {
    // Metadata parsed from a .torrent file.
    struct TorrentMeta {
        std::string announce;
        std::string name;
        long long length = 0;
        long long piece_length = 0;
        std::vector<std::array<std::uint8_t, 20>> piece_hashes;
        std::string info_bencoded;
        std::array<std::uint8_t, 20> info_hash_raw{};
        std::string info_hash_urlencoded;
    };

    // Parse a .torrent file at `path` into TorrentMeta.
    // This will:
    //  - read the file
    //  - decode bencode
    //  - extract announce, name, length, piece length, pieces
    //  - compute info_bencoded, info_hash_raw, info_hash_urlencoded
    TorrentMeta parse_torrent_file(const std::string& path);

}

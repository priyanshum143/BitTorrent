#include "torrent/torrent_meta.hpp"
#include "torrent/bencode.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace torrent {

    using json = nlohmann::json;

    TorrentMeta parse_torrent_file(const std::string& path) {
        TorrentMeta meta;

        // 1. Read the raw .torrent file
        std::string encoded_content = read_file(path);

        // 2. Decode top-level bencoded dictionary into json
        json decoded_content = decode_bencoded_value(encoded_content);

        // 3. Basic fields from top-level
        meta.announce = decoded_content["announce"].get<std::string>();

        json info = decoded_content["info"];
        meta.name         = info["name"].get<std::string>();
        meta.length       = info["length"].get<long long>();
        meta.piece_length = info["piece length"].get<long long>();

        // 4. Pieces → piece_hashes
        std::string pieces_raw = info["pieces"].get<std::string>();
        std::list<std::string> pieces_list = split_pieces(pieces_raw);

        meta.piece_hashes.clear();
        meta.piece_hashes.reserve(pieces_list.size());

        for (const auto& s : pieces_list) {
            std::array<std::uint8_t, 20> arr{};
            for (std::size_t i = 0; i < 20; ++i) {
                arr[i] = static_cast<std::uint8_t>(static_cast<unsigned char>(s[i]));
            }
            meta.piece_hashes.push_back(arr);
        }

        // 5. info_bencoded
        meta.info_bencoded = encode_bencode_value(info);

        // 6. info_hash_raw
        std::string raw = sha1_raw(meta.info_bencoded);
        for (std::size_t i = 0; i < 20; ++i) {
            meta.info_hash_raw[i] = static_cast<std::uint8_t>(
                static_cast<unsigned char>(raw[i])
            );
        }

        // 7. URL-encoded version for tracker
        meta.info_hash_urlencoded = percent_encode_bytes(raw);

        return meta;
    }

}

#include "torrent/torrent_meta.hpp"
#include "torrent/bencode.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace torrent {

    using json = nlohmann::json;

    TorrentMeta parse_torrent_file(const std::string& path) {
        // TODO: in next step, we will:
        //  1. read_file(path)
        //  2. decode_bencoded_value
        //  3. extract announce, info, etc.
        //  4. fill TorrentMeta fields

        TorrentMeta meta;
        (void) path;
        return meta;
    }

}

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "torrent/torrent_meta.hpp"

namespace torrent {
    struct Peer {
        std::string ip;
        std::uint16_t port;
    };

    struct TrackerResponse {
        int interval = 0;
        std::vector<Peer> peers;
    };

    // Ask the tracker for peers for this torrent.
    //
    // - meta: already-parsed torrent metadata (contains announce URL, info_hash_urlencoded, length)
    // - peer_id: 20-byte peer ID (you can pass your "122333..." string here)
    //
    // Throws std::runtime_error on HTTP failure or tracker failure.
    TrackerResponse request_peers(const TorrentMeta& meta, const std::string& peer_id);
}

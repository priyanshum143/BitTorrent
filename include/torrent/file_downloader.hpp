#pragma once

#include <string>
#include "torrent/torrent_meta.hpp"
#include "torrent/tracker.hpp"
#include "torrent/peer.hpp"

namespace torrent {

    // Download the entire file described by `meta` and write it to `output_path`.
    //
    // Simple version:
    //  - use a single peer (first from tracker)
    //  - download pieces sequentially: 0,1,2,...,N-1
    //  - verify each piece hash (optional now, easy later)
    //  - write in order to disk
    //
    // Throws std::runtime_error on any fatal error.
    void download_file_single_peer(
        const TorrentMeta& meta,
        const std::string& peer_id,
        const std::string& output_path
    );

}

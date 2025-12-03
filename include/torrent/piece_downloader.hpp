#pragma once

#include <cstdint>
#include <vector>

#include "torrent/torrent_meta.hpp"
#include "torrent/peer.hpp"

namespace torrent {

    std::uint32_t piece_size(const TorrentMeta& meta, std::uint32_t index);

    // Download a single piece from a connected peer.
    //
    // - conn_fd: socket fd from a successful PeerConnection
    // - meta: TorrentMeta
    // - piece_index: which piece to download
    //
    // Returns raw bytes of the piece (no file I/O here).
    // Throws on protocol/IO errors.
    std::vector<std::uint8_t> download_piece_from_peer(
        int conn_fd,
        const TorrentMeta& meta,
        std::uint32_t piece_index
    );

}

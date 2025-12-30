#include "torrent/file_downloader.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

#include "torrent/torrent_meta.hpp"
#include "torrent/tracker.hpp"
#include "torrent/peer.hpp"
#include "torrent/piece_downloader.hpp"

namespace torrent {

    void download_file_single_peer(
        const TorrentMeta& meta,
        const std::string& peer_id,
        const std::string& output_path
    ) {
        std::cerr << "Starting full-file download (single peer)...\n";

        // 1) Ask tracker for peers
        TrackerResponse tr = request_peers(meta, peer_id);
        if (tr.peers.empty()) {
            throw std::runtime_error("Tracker returned no peers");
        }

        const Peer& peer = tr.peers.front();
        std::cerr << "Using peer " << peer.ip << ":" << peer.port << "\n";

        // 2) Open output file
        std::fstream out(output_path,
                        std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open output file: " + output_path);
        }

        // Optionally pre-allocate file size (helps with some filesystems)
        if (meta.length > 0) {
            out.seekp(meta.length - 1, std::ios::beg);
            out.write("", 1);
            out.flush();
            out.seekp(0, std::ios::beg);
        }

        const std::uint32_t num_pieces =
            static_cast<std::uint32_t>(meta.piece_hashes.size());

        std::cerr << "File length  : " << meta.length << "\n";
        std::cerr << "Piece length : " << meta.piece_length << "\n";
        std::cerr << "Num pieces   : " << num_pieces << "\n";

        // 3) Download pieces sequentially from the same connection
        for (std::uint32_t piece_index = 0; piece_index < num_pieces; ++piece_index) {
            std::cerr << "[*] Downloading piece " << piece_index
                    << " / " << (num_pieces - 1) << "...\n";

            auto conn = PeerConnection::connect_and_handshake(meta, peer, peer_id);

            // Use existing low-level piece downloader
            std::vector<std::uint8_t> buf =
                download_piece_from_peer(conn->socket_fd(), meta, piece_index);

            // Determine expected length for this piece
            std::uint32_t expected_len = meta.piece_length;
            if (piece_index == num_pieces - 1) {
                // Last piece may be shorter
                std::uint64_t full_before_last =
                    static_cast<std::uint64_t>(meta.piece_length) * (num_pieces - 1);
                expected_len = static_cast<std::uint32_t>(meta.length - full_before_last);
            }

            if (buf.size() < expected_len) {
                throw std::runtime_error("Downloaded piece shorter than expected");
            }

            // Write this piece at the correct offset
            std::uint64_t offset =
                static_cast<std::uint64_t>(meta.piece_length) * piece_index;

            out.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
            if (!out) {
                throw std::runtime_error("seekp failed");
            }

            out.write(reinterpret_cast<const char*>(buf.data()),
                    static_cast<std::streamsize>(expected_len));
            if (!out) {
                throw std::runtime_error("write failed");
            }

            std::cerr << "[✓] Piece " << piece_index << " done\n";
        }

        std::cerr << "[✓] Download complete, saved to " << output_path << "\n";
    }

}

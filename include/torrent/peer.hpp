#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "torrent/torrent_meta.hpp"
#include "torrent/tracker.hpp"

namespace torrent {
    // Represents a TCP connection to a single BitTorrent peer.
    class PeerConnection {
    public:
        // Factory: create and connect to a peer, perform handshake.
        //
        // - meta: TorrentMeta, used for info_hash and name
        // - peer: Peer (ip + port) from tracker
        // - peer_id: this client's 20-byte peer_id string
        //
        // Throws std::runtime_error on connect/handshake failure.
        static std::unique_ptr<PeerConnection> connect_and_handshake(
            const TorrentMeta& meta,
            const Peer& peer,
            const std::string& peer_id
        );

        ~PeerConnection();

        std::string remote_ip() const { return m_peer.ip; }
        std::uint16_t remote_port() const { return m_peer.port; }
    private:
        PeerConnection(
            const TorrentMeta& meta,
            const Peer& peer,
            const std::string& peer_id
        );

        void perform_handshake(const TorrentMeta& meta, const std::string& peer_id);

        int m_socket_fd = -1;
        Peer m_peer;
    };

}

#include "torrent/peer.hpp"
#include "torrent/bencode.hpp"
#include "torrent/net_utils.hpp"
#include "torrent/string_utils.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace torrent {

    using Bytes20   = std::array<std::uint8_t, 20>;
    using Handshake = std::array<std::uint8_t, 68>;

    // Convert a std::string (length 20) to Bytes20
    static Bytes20 to_bytes20(const std::string& s) {
        if (s.size() != 20) {
            throw std::runtime_error("to_bytes20: string must be exactly 20 bytes");
        }
        Bytes20 out{};
        for (std::size_t i = 0; i < 20; ++i) {
            out[i] = static_cast<std::uint8_t>(static_cast<unsigned char>(s[i]));
        }
        return out;
    }

    static bool printable20(const std::uint8_t* data) {
        for (int i = 0; i < 20; ++i) {
            unsigned char c = data[i];
            if (c < 32 || c > 126) return false;
        }
        return true;
    }

    static Handshake build_handshake(const Bytes20& info_hash, const Bytes20& peer_id) {
        Handshake hs{};

        const std::string pstr = "BitTorrent protocol";
        const std::uint8_t pstrlen = static_cast<std::uint8_t>(pstr.size());

        hs[0] = pstrlen;
        std::memcpy(&hs[1], pstr.data(), pstr.size());
        std::memcpy(&hs[28], info_hash.data(), 20);
        std::memcpy(&hs[48], peer_id.data(), 20);

        return hs;
    }


    // ----------------- PeerConnection implementation -----------------

    PeerConnection::PeerConnection(
        const TorrentMeta& meta,
        const Peer& peer,
        const std::string& peer_id_ascii
    ): m_peer(peer) {
        // 1) Resolve and connect
        addrinfo hints{};
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family   = AF_UNSPEC;

        addrinfo* res = nullptr;
        int gai = ::getaddrinfo(
            peer.ip.c_str(),
            std::to_string(peer.port).c_str(),
            &hints,
            &res
        );
        if (gai != 0) {
            throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(gai));
        }

        int fd = -1;
        for (addrinfo* it = res; it; it = it->ai_next) {
            int s = ::socket(it->ai_family, it->ai_socktype, it->ai_protocol);
            if (s < 0) continue;

            // 5s timeouts like in your code
            timeval tv{};
            tv.tv_sec  = 5;
            tv.tv_usec = 0;
            ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            ::setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

            if (::connect(s, it->ai_addr, it->ai_addrlen) == 0) {
                fd = s;
                break;
            }
            ::close(s);
        }

        ::freeaddrinfo(res);

        if (fd < 0) {
            throw std::runtime_error("connect failed");
        }

        m_socket_fd = fd;

        // 2) Perform BitTorrent handshake (your logic moved to a method)
        perform_handshake(meta, peer_id_ascii);
    }

    PeerConnection::~PeerConnection() {
        if (m_socket_fd >= 0) {
            ::close(m_socket_fd);
            m_socket_fd = -1;
        }
    }

    std::unique_ptr<PeerConnection> PeerConnection::connect_and_handshake(
        const TorrentMeta& meta,
        const Peer& peer,
        const std::string& peer_id_ascii
    ) {
        return std::unique_ptr<PeerConnection>(
            new PeerConnection(meta, peer, peer_id_ascii)
        );
    }

    void PeerConnection::perform_handshake(
        const TorrentMeta& meta,
        const std::string& peer_id_ascii
    ) {
        if (peer_id_ascii.size() != 20) {
            throw std::runtime_error("peer_id must be exactly 20 bytes");
        }

        Bytes20 info_hash{};
        for (std::size_t i = 0; i < 20; ++i) {
            info_hash[i] = meta.info_hash_raw[i];
        }

        Bytes20 peer_id = to_bytes20(peer_id_ascii);
        Handshake out_hs = build_handshake(info_hash, peer_id);

        // Send our handshake
        write_all(m_socket_fd, out_hs.data(), out_hs.size());

        // Read peer's handshake
        Handshake in_hs{};
        read_exact(m_socket_fd, in_hs.data(), in_hs.size());

        // Validate protocol string
        if (in_hs[0] != 19 || std::memcmp(&in_hs[1], "BitTorrent protocol", 19) != 0) {
            throw std::runtime_error("invalid protocol string");
        }

        // Validate info_hash
        if (std::memcmp(&in_hs[28], info_hash.data(), 20) != 0) {
            throw std::runtime_error("info_hash mismatch");
        }

        // print peer ID
        const std::uint8_t* pid = &in_hs[48];
        if (printable20(pid)) {
            std::cout.write(reinterpret_cast<const char*>(pid), 20);
            std::cout << "\n";
        }
        else {
            auto hex = [](const std::uint8_t* d, std::size_t n) {
                std::ostringstream o;
                o << std::hex << std::setfill('0');
                for (std::size_t i = 0; i < n; ++i) {
                    o << std::setw(2) << unsigned(d[i]);
                }
                return o.str();
            };
            std::cout << "Peer ID: " << hex(pid, 20) << "\n";
        }

        std::cerr << "Handshake OK with " << m_peer.ip << ":" << m_peer.port << "\n";
    }
}

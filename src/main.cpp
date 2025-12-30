#include <iostream>
#include <string>
#include <stdexcept>

#include "torrent/bencode.hpp"
#include "torrent/torrent_meta.hpp"
#include "torrent/tracker.hpp"
#include "torrent/peer.hpp"
#include "torrent/string_utils.hpp"
#include "torrent/piece_downloader.hpp"
#include "torrent/file_downloader.hpp"


using namespace torrent;

static void print_usage(const char* prog) {
    std::cerr
        << "Usage:\n"
        << "  " << prog << " info <torrent_file>\n"
        << "  " << prog << " peers <torrent_file>\n"
        << "  " << prog << " handshake <torrent_file> <host:port>\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::string torrent_path = argv[2];

    const std::string peer_id = "12233344441223334444";

    try {
        if (command == "info") {
            TorrentMeta meta = parse_torrent_file(torrent_path);

            std::cout << "Tracker URL: "   << meta.announce << "\n";
            std::cout << "Name       : "   << meta.name << "\n";
            std::cout << "Length     : "   << meta.length << "\n";
            std::cout << "Piece len  : "   << meta.piece_length << "\n";
            std::cout << "Num pieces : "   << meta.piece_hashes.size() << "\n";

            std::string info_hash_hex = sha1(meta.info_bencoded);
            std::cout << "Info hash  : "   << info_hash_hex << "\n";
        }
        else if (command == "peers") {
            TorrentMeta meta = parse_torrent_file(torrent_path);
            TrackerResponse tr = request_peers(meta, peer_id);

            std::cout << "Got " << tr.peers.size() << " peers from tracker\n";
            for (const auto& p : tr.peers) {
                std::cout << p.ip << ":" << p.port << "\n";
            }
        }
        else if (command == "handshake") {
            if (argc < 4) {
                print_usage(argv[0]);
                return 1;
            }

            TorrentMeta meta = parse_torrent_file(torrent_path);

            // Parse <host:port> argument into a Peer
            std::string host, port_str;
            torrent::split_host_port(argv[3], host, port_str);
            std::uint16_t port = static_cast<std::uint16_t>(std::stoi(port_str));

            Peer peer{host, port};

            // This will:
            //  - open TCP
            //  - send handshake
            //  - read handshake
            //  - validate info_hash and print peer_id (inside perform_handshake)
            auto conn = PeerConnection::connect_and_handshake(meta, peer, peer_id);

            std::cerr << "Handshake successful with "
                      << conn->remote_ip() << ":" << conn->remote_port() << "\n";
        }
        else if (command == "download_piece") {
            if (argc < 5) {
                std::cerr << "Usage: " << argv[0] << " download_piece <torrent_file> <piece_index> <host:port>\n";
                return 1;
            }

            TorrentMeta meta = parse_torrent_file(torrent_path);

            std::uint32_t piece_index = static_cast<std::uint32_t>(std::stoul(argv[3]));

            std::string host, port_str;
            split_host_port(argv[4], host, port_str);
            std::uint16_t port = static_cast<std::uint16_t>(std::stoi(port_str));

            Peer peer{host, port};

            auto conn = PeerConnection::connect_and_handshake(meta, peer, peer_id);

            auto data = download_piece_from_peer(conn->socket_fd(), meta, piece_index);

            std::cout.write(reinterpret_cast<const char*>(data.data()), data.size());
        }
        else if (command == "download") {
            if (argc < 5 || std::string(argv[2]) != "-o") {
                std::cerr << "Usage: " << argv[0] << " download -o <output_path> <torrent_file>\n";
                return 1;
            }

            std::string output_path = argv[3];
            std::string torrent_path = argv[4];

            TorrentMeta meta = parse_torrent_file(torrent_path);

            const std::string peer_id = "12233344441223334444";

            download_file_single_peer(meta, peer_id, output_path);
        }
        else {
            print_usage(argv[0]);
            return 1;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

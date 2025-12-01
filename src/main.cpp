#include "torrent/torrent_meta.hpp"
#include "torrent/tracker.hpp"
#include "torrent/bencode.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bt_main <file_path>\n";
        return 1;
    }

    try {
        torrent::TorrentMeta meta = torrent::parse_torrent_file(argv[1]);

        std::cout << "Tracker URL: " << meta.announce << "\n";
        std::cout << "Length: "      << meta.length << "\n";

        std::string peer_id = "12233344441223334444";
        torrent::TrackerResponse tr = torrent::request_peers(meta, peer_id);

        std::cout << "Peers from tracker:\n";
        for (const auto& p : tr.peers) {
            std::cout << p.ip << ":" << p.port << "\n";
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

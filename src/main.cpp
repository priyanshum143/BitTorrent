#include "torrent/torrent_meta.hpp"
#include "torrent/bencode.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bt_main <file_path>\n";
        return 1;
    }

    std::string file_path = argv[1];

    try {
        torrent::TorrentMeta meta = torrent::parse_torrent_file(file_path);

        std::cout << "Tracker URL: "   << meta.announce << "\n";
        std::cout << "Length: "        << meta.length << "\n";

        std::string info_hash_hex = torrent::sha1(meta.info_bencoded);
        std::cout << "Info Hash: "    << info_hash_hex << "\n";

        std::cout << "Piece Length: "  << meta.piece_length << "\n";
        std::cout << "Num Pieces: "    << meta.piece_hashes.size() << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
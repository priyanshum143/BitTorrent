#pragma once

#include <string>
#include <list>
#include <nlohmann/json.hpp>

namespace torrent {

    using json = nlohmann::json;


    // Read an entire file into a string (used for .torrent files and tests)
    std::string read_file(const std::string& file_path);


    // --- Bencode decoding ---
    json decode_bencoded_value(const std::string& encoded_value);

    json decode_string_bencoded_value(const std::string& encoded_value);
    json decode_integer_bencoded_value(const std::string& encoded_value);
    json decode_list_bencoded_value(const std::string& encoded_value);
    json decode_dict_bencoded_value(const std::string& encoded_value);


    // --- Bencode encoding ---
    std::string encode_bencode_value(const json& value);


    // --- Hashing & helpers ---

    // Return hex-encoded SHA1 of the input.
    std::string sha1(const std::string& input);

    // Return *raw* 20-byte SHA1 result (binary) in a std::string.
    std::string sha1_raw(const std::string& input);

    // Percent-encode a sequence of bytes (for info_hash in tracker URL).
    std::string percent_encode_bytes(const std::string& bytes);

    // Split the `pieces` field (20-byte SHA1 hashes concatenated) into
    // individual 20-byte strings (still binary, not hex).
    std::list<std::string> split_pieces(const std::string& pieces);
}

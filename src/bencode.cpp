#include "torrent/bencode.hpp"

#include <openssl/sha.h>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <iomanip>

namespace torrent {

    using json = nlohmann::json;

    // ---------------- File I/O ----------------

    std::string read_file(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + file_path);
        }

        std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return file_content;
    }


    // ---------------- Decoding bencoded values ----------------

    json decode_bencoded_value(const std::string& encoded_value) {
        size_t encoded_value_length = encoded_value.length();

        if (std::isdigit(encoded_value[0])) {
            return decode_string_bencoded_value(encoded_value);
        } else if (encoded_value_length > 2 && encoded_value[0] == 'i' && encoded_value[encoded_value_length - 1] == 'e') {
            return decode_integer_bencoded_value(encoded_value);
        } else if (encoded_value_length > 1 && encoded_value[0] == 'l' && encoded_value[encoded_value_length - 1] == 'e') {
            return decode_list_bencoded_value(encoded_value);
        } else if (encoded_value_length > 1 && encoded_value[0] == 'd' && encoded_value[encoded_value_length - 1] == 'e') {
            return decode_dict_bencoded_value(encoded_value);
        } else {
            throw std::runtime_error("Unhandled encoded value: " + encoded_value);
        }
    }

    json decode_string_bencoded_value(const std::string& encoded_value) {
        size_t colon_index = encoded_value.find(':');
        if (colon_index != std::string::npos) {
            std::string number_string = encoded_value.substr(0, colon_index);
            int64_t number = std::atoll(number_string.c_str());
            std::string str = encoded_value.substr(colon_index + 1, number);
            return json(str);
        }
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    json decode_integer_bencoded_value(const std::string& encoded_value) {
        size_t encoded_value_length = encoded_value.length();

        size_t idx = 1;
        bool isNegative = false;
        if (encoded_value[idx] == '-') {
            if (encoded_value_length < 4) throw std::runtime_error("Invalid encoded value: " + encoded_value);

            idx++;
            isNegative = true;

            if (encoded_value[idx] == '0') throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }

        if (!isNegative && encoded_value[idx] == '0' && encoded_value_length > 3) {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }

        for (size_t i = idx; i < encoded_value.length() - 1; i++) {
            if (!(encoded_value[i] >= '0' && encoded_value[i] <= '9')) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }
        }

        std::string number_string = encoded_value.substr(1, encoded_value_length - 2);
        int64_t number = std::atoll(number_string.c_str());
        return json(number);
    }

    json decode_list_bencoded_value(const std::string& encoded_value) {
        size_t encoded_value_length = encoded_value.length();
        if (encoded_value_length == 2) return json::array();

        json decoded_list = json::array();

        size_t idx = 1;
        while (idx < encoded_value_length - 1 && encoded_value[idx] != 'e') {
            if (encoded_value[idx] == 'i') {
                size_t e_idx = idx;
                while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;

                size_t encoded_int_len = e_idx - idx + 1;
                std::string encoded_int = encoded_value.substr(idx, encoded_int_len);
                decoded_list.push_back(decode_integer_bencoded_value(encoded_int));

                idx = e_idx + 1;
            }
            else if (std::isdigit(encoded_value[idx])) {
                size_t colon_index = encoded_value.find(':', idx);
                if (colon_index != std::string::npos) {
                    std::string number_string = encoded_value.substr(idx, colon_index - idx);
                    int64_t number = std::atoll(number_string.c_str());
                    std::string str = encoded_value.substr(colon_index + 1, number);
                    decoded_list.push_back(str);

                    idx = colon_index + 1 + number;
                }
                else {
                    throw std::runtime_error("Invalid encoded value: " + encoded_value);
                }
            }
            else if (encoded_value[idx] == 'l') {
                std::string encoded_list = encoded_value.substr(idx);
                decoded_list.push_back(decode_list_bencoded_value(encoded_list));
                idx++;

                size_t l_count = 1;
                size_t e_count = 0;
                while (idx < encoded_value_length && l_count != e_count) {
                    if (encoded_value[idx] == 'i') {
                        size_t e_idx = idx;
                        while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;
                        idx = e_idx + 1;
                    }
                    else if (std::isdigit(encoded_value[idx])) {
                        size_t colon_index = encoded_value.find(':', idx);
                        std::string number_string = encoded_value.substr(idx, colon_index - idx);
                        int64_t number = std::atoll(number_string.c_str());
                        idx = colon_index + 1 + number;
                    }
                    else if (encoded_value[idx] == 'l') {
                        l_count++; idx++;
                    }
                    else if (encoded_value[idx] == 'e') {
                        e_count++; idx++;
                    }
                }
            }
            else {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }

        }
        return decoded_list;
    }

    json decode_dict_bencoded_value(const std::string& encoded_value) {
        size_t encoded_value_length = encoded_value.length();
        if (encoded_value_length == 2) return json::object();

        json decoded_dict = json::object();
        int64_t iteration = 0;
        std::string current_key = "";

        size_t idx = 1;
        while (idx < encoded_value_length - 1 && encoded_value[idx] != 'e') {
            iteration++;

            if (iteration % 2 == 1) {
                if (std::isdigit(encoded_value[idx])) {
                    size_t colon_index = encoded_value.find(':', idx);
                    if (colon_index != std::string::npos) {
                        std::string number_string = encoded_value.substr(idx, colon_index - idx);
                        int64_t number = std::atoll(number_string.c_str());
                        current_key = encoded_value.substr(colon_index + 1, number);
                        decoded_dict[current_key] = "";

                        idx = colon_index + 1 + number;
                    }
                    else {
                        throw std::runtime_error("Invalid encoded value: " + encoded_value);
                    }
                }
                else {
                    throw std::runtime_error("Invalid encoded value: " + encoded_value);
                }
            }
            else {
                if (std::isdigit(encoded_value[idx])) {
                    size_t colon_index = encoded_value.find(':', idx);
                    if (colon_index != std::string::npos) {
                        std::string number_string = encoded_value.substr(idx, colon_index - idx);
                        int64_t number = std::atoll(number_string.c_str());
                        std::string str = encoded_value.substr(colon_index + 1, number);
                        decoded_dict[current_key] = str;

                        idx = colon_index + 1 + number;
                    }
                    else {
                        throw std::runtime_error("Invalid encoded value: " + encoded_value);
                    }
                }
                else if (encoded_value[idx] == 'i') {
                    size_t e_idx = idx;
                    while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;

                    size_t encoded_int_len = e_idx - idx + 1;
                    std::string encoded_int = encoded_value.substr(idx, encoded_int_len);
                    decoded_dict[current_key] = decode_integer_bencoded_value(encoded_int);

                    idx = e_idx + 1;
                }
                else if (encoded_value[idx] == 'l') {
                    std::string encoded_list = encoded_value.substr(idx);
                    decoded_dict[current_key] = decode_list_bencoded_value(encoded_list);

                    idx++;

                    size_t l_count = 1;
                    size_t e_count = 0;
                    while (idx < encoded_value_length && l_count != e_count) {
                        if (encoded_value[idx] == 'i') {
                            size_t e_idx = idx;
                            while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;
                            idx = e_idx + 1;
                        }
                        else if (std::isdigit(encoded_value[idx])) {
                            size_t colon_index = encoded_value.find(':', idx);
                            std::string number_string = encoded_value.substr(idx, colon_index - idx);
                            int64_t number = std::atoll(number_string.c_str());
                            idx = colon_index + 1 + number;
                        }
                        else if (encoded_value[idx] == 'l') {
                            l_count++; idx++;
                        }
                        else if (encoded_value[idx] == 'e') {
                            e_count++; idx++;
                        }
                    }
                }
                else if (encoded_value[idx] == 'd') {
                    std::string encoded_dict = encoded_value.substr(idx);
                    decoded_dict[current_key] = decode_dict_bencoded_value(encoded_dict);
                    idx++;

                    size_t d_count = 1;
                    size_t e_count = 0;
                    while (idx < encoded_value_length && d_count != e_count) {
                        if (encoded_value[idx] == 'i') {
                            size_t e_idx = idx;
                            while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;
                            idx = e_idx + 1;
                        }
                        else if (std::isdigit(encoded_value[idx])) {
                            size_t colon_index = encoded_value.find(':', idx);
                            std::string number_string = encoded_value.substr(idx, colon_index - idx);
                            int64_t number = std::atoll(number_string.c_str());
                            idx = colon_index + 1 + number;
                        }
                        else if (encoded_value[idx] == 'l') {
                            idx++;

                            size_t l_count = 1;
                            size_t e_count_for_l = 0;
                            while (idx < encoded_value_length && l_count != e_count_for_l) {
                                if (encoded_value[idx] == 'i') {
                                    size_t e_idx = idx;
                                    while (e_idx < encoded_value_length && encoded_value[e_idx] != 'e') e_idx++;
                                    idx = e_idx + 1;
                                }
                                else if (std::isdigit(encoded_value[idx])) {
                                    size_t colon_index = encoded_value.find(':', idx);
                                    std::string number_string = encoded_value.substr(idx, colon_index - idx);
                                    int64_t number = std::atoll(number_string.c_str());
                                    idx = colon_index + 1 + number;
                                }
                                else if (encoded_value[idx] == 'l') {
                                    l_count++; idx++;
                                }
                                else if (encoded_value[idx] == 'e') {
                                    e_count_for_l++; idx++;
                                }
                            }
                        }
                        else if (encoded_value[idx] == 'd') {
                            d_count++; idx++;
                        }
                        else if (encoded_value[idx] == 'e') {
                            e_count++; idx++;
                        }
                    }
                }
                else {
                    throw std::runtime_error("Invalid encoded value: " + encoded_value);
                }
            }
        }
        return decoded_dict;
    }


    // ---------------- Encode info hash in bencoded ----------------

    std::string encode_bencode_value(const json& value) {
        json::value_t type = value.type();
        if (type == json::value_t::string) {
            std::string str = value.get<std::string>();
            return std::to_string(str.size()) + ":" + str;
        }
        else if (type == json::value_t::number_integer) {
            return "i" + std::to_string(value.get<int64_t>()) + "e";
        }
        else if (type == json::value_t::array) {
            std::string result = "l";
            for (const auto& item : value) {
                result += encode_bencode_value(item);
            }
            return result + "e";
        }
        else if (type == json::value_t::object) {
            std::string result = "d";
            for (auto it = value.begin(); it != value.end(); ++it) {
                result += encode_bencode_value(it.key()) + encode_bencode_value(it.value());
            }
            return result + "e";
        }

        return "";
    }


    // ---------------- Hashing and Helpers ----------------

    std::string sha1(const std::string& input) {
        std::string raw = sha1_raw(input);

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned char c : raw) {
            oss << std::setw(2) << static_cast<int>(c);
        }
        return oss.str();
    }

    std::string sha1_raw(const std::string& input) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(input.data()),
            input.size(),
            hash);

        return std::string(reinterpret_cast<const char*>(hash), SHA_DIGEST_LENGTH); // 20 bytes
    }

    std::string percent_encode_bytes(const std::string& bytes) {
        std::ostringstream oss;
        oss << std::uppercase << std::hex;

        for (unsigned char c : bytes) {
            oss << '%' << std::setw(2) << std::setfill('0') << int(c);
        }

        return oss.str();
    }

    std::list<std::string> split_pieces(const std::string& pieces) {
        std::list<std::string> result;
        for (size_t i = 0; i < pieces.length(); i += 20) {
            std::string piece = pieces.substr(i, 20);
            // hexlify the piece
            std::stringstream ss;
            for (unsigned char piece_char : piece) {
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(piece_char);
            }
            result.push_back(ss.str());
        }
        return result;
    }
}

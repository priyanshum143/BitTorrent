#pragma once

#include <string>
#include <stdexcept>

namespace torrent {

    inline void split_host_port(const std::string& input, std::string& host, std::string& port_str) {
        auto pos = input.find(':');
        if (pos == std::string::npos) {
            throw std::runtime_error("Expected <host>:<port>, got: " + input);
        }
        host = input.substr(0, pos);
        port_str = input.substr(pos + 1);
    }
}

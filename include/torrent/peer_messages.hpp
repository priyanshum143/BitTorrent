#pragma once

#include <cstdint>
#include <vector>
#include <optional>

#include <arpa/inet.h>

namespace torrent {

    enum class MsgId : std::uint8_t {
        Choke         = 0,
        Unchoke       = 1,
        Interested    = 2,
        NotInterested = 3,
        Have          = 4,
        Bitfield      = 5,
        Request       = 6,
        Piece         = 7,
        Cancel        = 8,
        Port          = 9
    };

    struct BtMessage {
        std::uint32_t length = 0;
        std::optional<MsgId> id;
        std::vector<std::uint8_t> payload;
    };

    std::vector<std::uint8_t> build_interested();

    std::vector<std::uint8_t> build_request(
        std::uint32_t piece_index,
        std::uint32_t begin,
        std::uint32_t length
    );

    BtMessage read_message(int sock_fd);

}

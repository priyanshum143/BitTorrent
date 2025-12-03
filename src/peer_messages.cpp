#include "torrent/peer_messages.hpp"
#include "torrent/net_utils.hpp"

#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace torrent {

    std::vector<std::uint8_t> build_interested() {
        // length = 1 (just the message ID), id = 2
        std::vector<std::uint8_t> msg;
        msg.resize(4 + 1);

        // length prefix (big-endian)
        std::uint32_t len = htonl(1);
        std::memcpy(msg.data(), &len, 4);

        msg[4] = static_cast<std::uint8_t>(MsgId::Interested);
        return msg;
    }

    std::vector<std::uint8_t> build_request(
        std::uint32_t piece_index,
        std::uint32_t begin,
        std::uint32_t length
    ) {
        std::vector<std::uint8_t> msg;
        msg.resize(4 + 1 + 12);

        std::uint32_t len = htonl(13);
        std::memcpy(msg.data(), &len, 4);

        msg[4] = static_cast<std::uint8_t>(MsgId::Request);

        std::uint32_t be_index  = htonl(piece_index);
        std::uint32_t be_begin  = htonl(begin);
        std::uint32_t be_length = htonl(length);

        std::memcpy(msg.data() + 5,  &be_index,  4);
        std::memcpy(msg.data() + 9,  &be_begin,  4);
        std::memcpy(msg.data() + 13, &be_length, 4);

        return msg;
    }

    BtMessage read_message(int sock_fd) {
        BtMessage out;

        // Read 4-byte length prefix
        std::uint32_t len_be = 0;
        read_exact(sock_fd, &len_be, 4);
        out.length = ntohl(len_be);

        if (out.length == 0) {
            // keep-alive
            out.id.reset();
            out.payload.clear();
            return out;
        }

        // Read [id][payload...]
        std::vector<std::uint8_t> buf(out.length);
        read_exact(sock_fd, buf.data(), buf.size());

        out.id = static_cast<MsgId>(buf[0]);
        if (out.length > 1) {
            out.payload.assign(buf.begin() + 1, buf.end());
        }

        return out;
    }

}

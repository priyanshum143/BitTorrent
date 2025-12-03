#include "torrent/piece_downloader.hpp"
#include "torrent/peer_messages.hpp"

#include <stdexcept>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace torrent {

    std::uint32_t piece_size(const TorrentMeta& meta, std::uint32_t index) {
        std::uint64_t full = meta.piece_length;
        std::uint64_t total = meta.length;
        std::uint64_t num_pieces = meta.piece_hashes.size();

        if (index >= num_pieces) {
            throw std::runtime_error("piece_size: index out of range");
        }

        if (index < num_pieces - 1) {
            return static_cast<std::uint32_t>(full);
        }

        std::uint64_t last_size = total - full * (num_pieces - 1);
        return static_cast<std::uint32_t>(last_size);
    }

    static void send_msg(int fd, const std::vector<std::uint8_t>& msg) {
        std::size_t total = 0;
        while (total < msg.size()) {
            ssize_t n = ::send(fd, msg.data() + total, msg.size() - total, 0);
            if (n <= 0) throw std::runtime_error("send_msg failed");
            total += static_cast<std::size_t>(n);
        }
    }

    std::vector<std::uint8_t> download_piece_from_peer(
        int conn_fd,
        const TorrentMeta& meta,
        std::uint32_t piece_index
    ) {
        const std::uint32_t block_size = 16 * 1024; // 16 KiB, same as Codecrafters
        const std::uint32_t ps = piece_size(meta, piece_index);

        std::vector<std::uint8_t> piece(ps);

        // 1. Send "interested"
        auto interested = build_interested();
        send_msg(conn_fd, interested);

        // 2. Wait for "unchoke"
        bool unchoked = false;
        while (!unchoked) {
            BtMessage msg = read_message(conn_fd);
            if (!msg.id) {
                // keep-alive; ignore
                continue;
            }
            switch (*msg.id) {
                case MsgId::Unchoke:
                    unchoked = true;
                    break;
                case MsgId::Choke:
                    throw std::runtime_error("peer choked us");
                default:
                    // ignore everything else here for now
                    break;
            }
        }

        // 3. Request all blocks in this piece
        for (std::uint32_t offset = 0; offset < ps; offset += block_size) {
            std::uint32_t req_len = std::min(block_size, ps - offset);
            auto req = build_request(piece_index, offset, req_len);
            send_msg(conn_fd, req);
        }

        // 4. Read "piece" messages until we've filled the buffer
        std::uint32_t bytes_received = 0;
        while (bytes_received < ps) {
            BtMessage msg = read_message(conn_fd);
            if (!msg.id) continue; // keep-alive

            if (*msg.id == MsgId::Piece) {
                // payload: [index(4)][begin(4)][block...]
                if (msg.payload.size() < 8) {
                    throw std::runtime_error("piece message too short");
                }

                std::uint32_t idx, begin;
                std::memcpy(&idx,   msg.payload.data(),      4);
                std::memcpy(&begin, msg.payload.data() + 4,  4);
                idx   = ntohl(idx);
                begin = ntohl(begin);

                if (idx != piece_index) {
                    // piece from some other index; ignore for now
                    continue;
                }

                std::size_t block_len = msg.payload.size() - 8;
                if (begin + block_len > piece.size()) {
                    throw std::runtime_error("piece block out of range");
                }

                std::memcpy(piece.data() + begin,
                            msg.payload.data() + 8,
                            block_len);

                bytes_received += static_cast<std::uint32_t>(block_len);
            }
            else if (*msg.id == MsgId::Choke) {
                throw std::runtime_error("peer choked mid-piece");
            }
            else {
                // ignore other message types for this stage
            }
        }

        return piece;
    }

}

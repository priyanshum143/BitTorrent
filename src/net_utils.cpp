#include "torrent/net_utils.hpp"

#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace torrent {

    void write_all(int fd, const void* buf, std::size_t len) {
        const std::uint8_t* p = static_cast<const std::uint8_t*>(buf);
        std::size_t total = 0;
        while (total < len) {
            ssize_t n = ::send(fd, p + total, len - total, 0);
            if (n <= 0) throw std::runtime_error("write_all: send failed");
            total += static_cast<std::size_t>(n);
        }
    }

    void read_exact(int fd, void* buf, std::size_t len) {
        std::uint8_t* p = static_cast<std::uint8_t*>(buf);
        std::size_t total = 0;
        while (total < len) {
            ssize_t n = ::recv(fd, p + total, len - total, 0);
            if (n <= 0) throw std::runtime_error("read_exact: recv failed");
            total += static_cast<std::size_t>(n);
        }
    }

}

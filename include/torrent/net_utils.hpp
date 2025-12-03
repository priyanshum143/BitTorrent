#pragma once

#include <cstddef>
#include <cstdint>

namespace torrent {

    void write_all(int fd, const void* buf, std::size_t len);
    void read_exact(int fd, void* buf, std::size_t len);

}

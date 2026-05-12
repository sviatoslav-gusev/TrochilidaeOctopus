#pragma once

#include <cstdint>

namespace gs::network {

struct SeqNums {
    uint32_t expected_rx_seq{0};
    uint32_t tx_seq{0};

    void Reset() {
        expected_rx_seq = 0;
        tx_seq = 0;
    }
};

} // namespace gs::network

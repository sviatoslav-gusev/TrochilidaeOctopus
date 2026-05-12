#pragma once

#include "shared/protocol/MsgHeader.h"
#include <concepts>

namespace gs::traits {

template <typename T>
concept WithHeader = requires(T t) {
    { t.header } -> std::same_as<protocol::MsgHeader&>;
    { t.header.type } -> std::same_as<enums::MsgType&>;
    { t.header.client_id } -> std::assignable_from<uint32_t>;
    { t.header.timestamp } -> std::assignable_from<int64_t>;
    { t.header.seq_num } -> std::assignable_from<uint32_t>;
};

} // namespace gs::traits

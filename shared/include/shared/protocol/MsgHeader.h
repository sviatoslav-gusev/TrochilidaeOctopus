#pragma once

#include "shared/Enums.h"

#include <QJsonObject>

#include <cstdint>

namespace gs::protocol {

struct MsgHeader {
    Q_GADGET

public:
    Q_PROPERTY(MsgType  type      MEMBER type)
    Q_PROPERTY(uint32_t client_id MEMBER client_id)
    Q_PROPERTY(uint32_t seq_num   MEMBER seq_num)
    Q_PROPERTY(int64_t  timestamp MEMBER timestamp)

    enums::MsgType  type{enums::MsgType::Unknown};
    uint32_t        client_id{0};
    uint32_t        seq_num{0};
    int64_t         timestamp{0};
};

} // namespace gs::protocol

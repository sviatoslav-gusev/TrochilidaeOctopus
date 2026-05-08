#pragma once

#include "shared/Enums.h"
#include "shared/protocol/MsgHeader.h"

#include <QJsonObject>
#include <QString>

/**            --- LOGIN/CHALLENGE FLOW ---
 *  0. Both client and server has some token.
 *  1. Client connects and send LoginRequest.
 *  2. Server gens random string (Nonce). Sends it inside ChallengeRequest.
 *  3. Client gets token, merge it with nonce ans make hash of this pair. Sends in ChallengeResponse.
 *  4. Server calculates same hash too.
 *  5a. Hash is same? Send enums::ConnectionStatus::Online in LoginResponse
 *  5b. Hash differs? Send enums::ConnectionStatus::Offline in LoginResponse
 */

namespace gs::protocol {

struct Heartbeat {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)

    MsgHeader header{enums::MsgType::Heartbeat};
};

// Client -> Server
struct LoginRequest {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)

    MsgHeader header{enums::MsgType::LoginRequest};
};

// Server -> Client
struct ChallengeRequest {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)
    Q_PROPERTY(QString   nonce  MEMBER nonce)

    MsgHeader header{enums::MsgType::ChallengeRequest};
    QString   nonce;
};

// Client -> Server
struct ChallengeResponse {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)
    Q_PROPERTY(QString   hash   MEMBER hash)

    MsgHeader header{enums::MsgType::ChallengeResponse};
    QString   hash;
};

// Server -> Client
struct LoginResponse {
    Q_GADGET

    Q_PROPERTY(MsgHeader               header      MEMBER header)
    Q_PROPERTY(enums::ConnectionStatus conn_status MEMBER conn_status)

    MsgHeader header{enums::MsgType::LoginResponse};
    enums::ConnectionStatus conn_status{enums::ConnectionStatus::Offline};
};

// struct LogoutRequest

// struct LogoutResponse

} // namespace gs::protocol

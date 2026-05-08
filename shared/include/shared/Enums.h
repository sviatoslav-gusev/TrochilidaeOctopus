#pragma once

#include <QJsonObject>

namespace gs::enums {
    Q_NAMESPACE

enum class MsgType {
    Unknown = 0,

    // --- System / Infrastructure ---
    Heartbeat,
    Log,

    // --- Auth (Client -> Server) ---
    LoginRequest,
      ChallengeRequest, // (Server -> Client)
      ChallengeResponse,
    LoginResponse,

    LogoutRequest,
    LogoutResponse,

    // --- Telemetry (Client -> Server) ---
    NetworkMetrics,

    // --- State Management (Server -> Client) ---
    StatusGetRequest,
    StatusGetResponse,
    StatusSetRequest,
    StatusSetResponse
};
Q_ENUM_NS(MsgType)

enum class ConnectionStatus {
    Offline = 0,
    Online
};
Q_ENUM_NS(ConnectionStatus)

enum class RunningStatus {
    Stopped = 0,
    Running
};
Q_ENUM_NS(RunningStatus)

enum class ExecStatus {
    Demo = 0,
    Ping
};
Q_ENUM_NS(ExecStatus)

} // namespace gs::enums

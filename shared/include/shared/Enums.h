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
    NetworkMetricsReport,

    // --- State Management (Server -> Client) ---
    SettingsGetRequest,
    SettingsGetResponse,
    SettingsSetRequest,
    SettingsSetResponse
};
Q_ENUM_NS(MsgType)

enum class ConnectionState {
    Offline = 0,
    Online
};
Q_ENUM_NS(ConnectionState)

enum class RunningState {
    Stopped = 0,
    Running
};
Q_ENUM_NS(RunningState)

enum class ExecMode {
    Demo = 0,
    Ping
};
Q_ENUM_NS(ExecMode)

} // namespace gs::enums

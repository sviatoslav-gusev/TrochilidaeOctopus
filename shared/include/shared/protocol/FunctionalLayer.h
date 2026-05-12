#pragma once

#include "shared/Enums.h"
#include "shared/protocol/MsgHeader.h"
#include "shared/protocol/ClientSettings.h"
#include "shared/protocol/NetworkMetrics.h"

#include <QJsonObject>
#include <QString>

namespace gs::protocol {

// Client -> Server
struct Log {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::Log;

    Q_PROPERTY(MsgHeader header MEMBER header)
    Q_PROPERTY(QString   msg    MEMBER msg)

    MsgHeader header{enums::MsgType::Log};
    QString   msg;
};

// Client -> Server
struct NetworkMetricsReport {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::NetworkMetricsReport;

    Q_PROPERTY(MsgHeader header        MEMBER header)
    Q_PROPERTY(NetworkMetrics metrics MEMBER metrics)

    MsgHeader      header{enums::MsgType::NetworkMetricsReport};
    NetworkMetrics metrics;
};

// Server -> Client
struct SettingsGetRequest {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::SettingsGetRequest;

    Q_PROPERTY(MsgHeader header MEMBER header)

    MsgHeader header{enums::MsgType::SettingsGetRequest};
};

// Client -> Server
struct SettingsGetResponse {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::SettingsGetResponse;

    Q_PROPERTY(MsgHeader      header          MEMBER header)
    Q_PROPERTY(ClientSettings client_settings MEMBER client_settings)

    MsgHeader      header{enums::MsgType::SettingsGetResponse};
    ClientSettings client_settings;
};

// Server -> Client
struct SettingsSetRequest {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::SettingsSetRequest;

    Q_PROPERTY(MsgHeader      header          MEMBER header)
    Q_PROPERTY(ClientSettings client_settings MEMBER client_settings)

    MsgHeader      header{enums::MsgType::SettingsSetRequest};
    ClientSettings client_settings;
};

// Client -> Server
struct SettingsSetResponse {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::SettingsSetResponse;

    Q_PROPERTY(MsgHeader      header          MEMBER header)
    Q_PROPERTY(ClientSettings client_settings MEMBER client_settings)

    MsgHeader      header{enums::MsgType::SettingsSetResponse};
    ClientSettings client_settings;
};

} // namespace gs::protocol

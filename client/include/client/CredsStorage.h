#pragma once

#include <QCommandLineParser>
#include <QSettings>
#include <QString>

namespace gs::client {

class CredsStorage {

public:
    CredsStorage();

    bool LoadCreds(QCoreApplication& app);

    uint32_t ClientID() const { return m_client_id; }
    QString Token() const { return m_token; }

private:
    static QString GetSettingsPath();

    bool LoadFromSettings();
    void SaveToSettings();

    void SetClientID(uint32_t id) { m_client_id = id; }
    void SetToken(const QString& t) { m_token = t; }

    bool IsValid() const { return m_client_id > 0 && !m_token.isEmpty(); }

    constexpr static QLatin1StringView CREDS{"credentials"};
    constexpr static QLatin1StringView CL_ID{"client_id"};
    constexpr static QLatin1StringView TOKEN{"token"};

    uint32_t  m_client_id;
    QString   m_token;
    QSettings m_settings;
};

} // namespace gs::client

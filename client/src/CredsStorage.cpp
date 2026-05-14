#include "client/CredsStorage.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

namespace gs::client {

CredsStorage::CredsStorage()
    : m_settings(GetSettingsPath(), QSettings::IniFormat)
{}

bool CredsStorage::LoadCreds(QCoreApplication& app) {
    // Try load
    LoadFromSettings();

    QCommandLineParser parser;
    // Parce args from cli
    parser.setApplicationDescription("Trochilidae Octopus client");
    parser.addHelpOption();

    const QCommandLineOption id_option({"i", "id"}, "Client ID", "id");
    const QCommandLineOption token_option({"t", "token"}, "Auth Token", "token");
    parser.addOption(id_option);
    parser.addOption(token_option);
    parser.process(app);

    bool cli_override = false;
    if (parser.isSet(id_option)) {
        cli_override = true;
        SetClientID(parser.value(id_option).toUInt());
    }
    if (parser.isSet(token_option)) {
        cli_override = true;
        SetToken(parser.value(token_option));
    }

    // Validate
    if (!IsValid()) {
        qCritical() << "Error: No ClientID or Token provided! Use --id and --token arguments.";
        return false;
    }

    if (cli_override) {
        SaveToSettings();
    }

    return true;
}

QString CredsStorage::GetSettingsPath() {
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    return path + "/settings.ini";
}

bool CredsStorage::LoadFromSettings() {

    m_settings.beginGroup(CREDS);

    // value(field name, default value)
    m_client_id = m_settings.value(CL_ID, 0).toUInt();
    m_token = m_settings.value(TOKEN, QString()).toString();

    m_settings.endGroup();

    qInfo() << "[Storage] Loaded credentials. ID:" << m_client_id
            << (m_token.isEmpty() ? " (Token missing)" : " (Token present)");

    return IsValid();
}

void CredsStorage::SaveToSettings() {   
    m_settings.beginGroup(CREDS);

    m_settings.setValue(CL_ID, m_client_id);
    m_settings.setValue(TOKEN, m_token);

    m_settings.endGroup();

    // force saving QSettings
    m_settings.sync();

    qInfo() << "[Storage] Credentials saved to persistent storage.";
}

} // namespace gs::client

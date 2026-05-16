#include "server/AppServer.h"
#include "server/DatabaseManager.h"
#include "shared/Enums.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QStyleHints>

int main(int argc, char *argv[]) {
    qSetMessagePattern("[%{time hh:mm:ss.zzz}] [%{type}] %{message}");

    QGuiApplication app(argc, argv);
    app.setOrganizationName("ExperimentalConnectivitySolutions");
    app.setApplicationName("OctopusServer");
    app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);

    if (!gs::server::DatabaseManager::instance().OpenDB("server_data.db")) {
        return -1;
    }

    //gs::server::DatabaseManager::instance().ClearAllData();

    gs::server::DatabaseManager::instance().AddClient(
        {.client_id = 13,
         .token = "WonderToken",
         .client_name = "Sviatoslav PC",
         .target = "127.0.0.1",
         .exec_mode = gs::enums::ExecMode::Demo,
         .run_state =  gs::enums::RunningState::Running,
         .ping_timeout_ms = 480,
         .metrics_report_interval_s = 8});

    gs::server::AppServer server;
    if (!server.StartServer(12345)) {
        return -1;
    }

    QQmlApplicationEngine engine;
    qmlRegisterUncreatableMetaObject(
        gs::enums::staticMetaObject,
        "gs.OctopusServer", 1, 0,
        "Enums",
        "Error: Enums is a namespace"
    );
    engine.rootContext()->setContextProperty("clientsModel", server.model());
    engine.rootContext()->setContextProperty("appServer", &server);

    const QUrl url(QStringLiteral("qrc:/gs/OctopusServer/src/ui/Main.qml"));
    engine.load(url);

    return app.exec();
}

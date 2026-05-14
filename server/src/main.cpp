#include <QCoreApplication>
#include "server/AppServer.h"
#include "server/DatabaseManager.h"

using namespace gs::protocol;

int main(int argc, char *argv[]) {
    qSetMessagePattern("[%{time hh:mm:ss.zzz}] [%{type}] %{message}");

    QCoreApplication app(argc, argv);
    app.setOrganizationName("ExperimentalConnectivitySolutions");
    app.setApplicationName("OctopusServer");

    if (!gs::server::DatabaseManager::instance().OpenDB("server_data.db")) {
        return -1;
    }

    // gs::server::DatabaseManager::instance().clearAllData();
    gs::server::DatabaseManager::instance().AddClient(13, "WonderToken", "Sviatoslav PC");

    gs::server::AppServer server;

    if (!server.StartServer(12345)) {
        return -1;
    }

    return app.exec();
}

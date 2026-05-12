#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>

#include "client/AppClient.h"
#include "client/CredsStorage.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setOrganizationName("ExperimentalConnectivitySolutions");
    app.setApplicationName("OctopusClient");

    gs::client::CredsStorage storage;
    if (!storage.LoadCreds(app)) {
        return -1;
    }

    gs::client::AppClient client(storage);
    return app.exec();
}

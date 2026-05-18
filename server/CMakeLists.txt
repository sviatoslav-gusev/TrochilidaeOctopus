project(OctopusServer VERSION 0.9)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Sql
    Gui
    Qml
    Quick
)

qt_add_executable(OctopusServer)

qt_add_qml_module(OctopusServer
    URI gs.OctopusServer
    SOURCES
        src/main.cpp

        include/server/ClientSession.h
        src/ClientSession.cpp

        include/server/AppServer.h
        src/AppServer.cpp

        include/server/DatabaseManager.h
        src/DatabaseManager.cpp

        include/server/models/ClientsModel.h
        src/models/ClientsModel.cpp

        include/server/Enums.h
        src/Enums.cpp

    QML_FILES
        src/ui/Main.qml
        QML_FILES src/ui/ClientEditorDialog.qml
)

target_include_directories(SharedLib PUBLIC
    include
)

target_link_libraries(OctopusServer PRIVATE
    SharedLib
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::Quick
)

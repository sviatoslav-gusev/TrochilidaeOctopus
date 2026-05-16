project(OctopusServer)

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
    VERSION 1.0
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

    QML_FILES
        src/ui/Main.qml
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

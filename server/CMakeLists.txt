project(server)

add_executable(server
    src/main.cpp

    include/server/ClientSession.h
    src/ClientSession.cpp

    include/server/AppServer.h
    src/AppServer.cpp
    include/server/DatabaseManager.h
    src/DatabaseManager.cpp
)

target_include_directories(SharedLib PUBLIC
    include
)

target_link_libraries(server PRIVATE
    SharedLib
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)

find_package(Qt6 REQUIRED COMPONENTS
    Sql
)

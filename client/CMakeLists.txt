project(OctopusClient VERSION 0.9)

add_executable(OctopusClient
    src/main.cpp

    include/client/AppClient.h
    src/AppClient.cpp
    include/client/CredsStorage.h
    src/CredsStorage.cpp

    include/client/engines/AbstractPingEngine.h

    src/engines/DemoPingEngine.cpp
    include/client/engines/DemoPingEngine.h
)

target_include_directories(SharedLib PUBLIC
    include
)

target_link_libraries(OctopusClient PRIVATE
    SharedLib
    Qt6::Core
    Qt6::Network
)

add_library(SharedLib STATIC
    include/shared/JsonSerializer.h
    src/JsonSerializer.cpp

    include/shared/Enums.h

    include/shared/protocol/MsgHeader.h
    include/shared/protocol/ClientStatus.h
    include/shared/protocol/NetworkMetrics.h

    include/shared/protocol/SessionLayer.h
    include/shared/protocol/FunctionalLayer.h

    include/shared/MessageDispatcher.h

    include/shared/JsonPacketStreamer.h
    src/JsonPacketStreamer.cpp
)

# headers for outer usage
target_include_directories(SharedLib PUBLIC
    include
)

# QtCore for QJson usage
target_link_libraries(SharedLib PUBLIC
    Qt6::Core
    Qt6::Network
)

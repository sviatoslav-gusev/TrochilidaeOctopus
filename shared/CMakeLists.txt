add_library(ProtocolLib STATIC
    include/shared/JsonSerializer.h
    src/JsonSerializer.cpp

    include/shared/Enums.h

    include/shared/protocol/MsgHeader.h
    include/shared/protocol/SessionLayer.h
    include/shared/protocol/FunctionalLayer.h
)

# headers for outer usage
target_include_directories(ProtocolLib PUBLIC include)

# QtCore for QJson usage
target_link_libraries(ProtocolLib PUBLIC Qt6::Core)

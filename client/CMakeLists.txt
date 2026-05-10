project(client)

add_executable(client
    main.cpp
)

target_link_libraries(client PRIVATE
    SharedLib
    Qt6::Core
    Qt6::Network
)
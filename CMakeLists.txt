cmake_minimum_required(VERSION 3.16)

project(TrochilidaeOctopus
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt standard for all subs
find_package(Qt6 6.5.2 REQUIRED COMPONENTS
    Core
    Network
)

# Subdirs, subprojects
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)

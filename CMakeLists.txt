cmake_minimum_required(VERSION 3.16)

project(TrochilidaeOctopus
    VERSION 1.0
    LANGUAGES CXX
)

# Qt standard for all subs
find_package(Qt6 6.5.2 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Widgets
)

# Subdirs, subprojects
add_subdirectory(shared)
cmake_minimum_required(VERSION 3.15)

project(epcphy VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(epcphy
    src/main.cpp
    src/reader.hpp
    src/reader.cpp
    src/gui.hpp
    src/gui.cpp
    src/crc/crc.cpp
    src/crc/crc.hpp
    src/crc/crc5epc_c1g2.h
    src/crc/crc5epc_c1g2.c
    src/crc/crc16genibus.h
    src/crc/crc16genibus.c
)

target_link_libraries(epcphy PRIVATE Qt6::Widgets)

set_target_properties(epcphy PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

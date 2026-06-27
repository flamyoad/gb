cmake_minimum_required(VERSION 4.1)
project(gb)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.5.0
)

FetchContent_MakeAvailable(CLI11)

add_subdirectory(cpu)

add_executable(gb main.cpp
        common_types.h
        mmu.h
        mmu.cpp
        gameboy.h
        gameboy.cpp
        bios.h
        timer.h
        timer.cpp
        serial.cpp
        serial.h
)

set_target_properties(gb PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(gb PRIVATE
        cpu
        CLI11::CLI11
)

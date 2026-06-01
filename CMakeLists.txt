cmake_minimum_required(VERSION 4.1)
project(gb)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(cpu)

add_executable(gb main.cpp
        common_types.h
        mmu.h
        mmu.cpp
        gameboy.h
        gameboy.cpp
        bios.h
        timer.h
        timer.cpp)

target_link_libraries(gb PRIVATE cpu)

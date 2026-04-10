add_library(cpu
        cpu.h
        cpu.cpp
        register.h
        register.cpp)

target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

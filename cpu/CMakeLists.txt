add_library(cpu
        cpu.h
        cpu.cpp
        register.h
        register.cpp
        flag_condition.h
        interrupt.h
)

target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

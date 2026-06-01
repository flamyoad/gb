//
// Created by Ng Zhen Hao on 30/05/2026.
//

#pragma once

#include "common_types.h"

class Gameboy;

class Timer {
public:
    explicit Timer(Gameboy &gb);

    static constexpr u32 CPU_FREQUENCY = 4194304; // 4.194304 MHz

    auto tick(u32 cycle) -> void;

    auto get_div() -> u8;
    auto get_tima() -> u8;
    auto get_tma() -> u8;
    auto get_tac() -> u8;

    auto set_div(u8 value) -> void;
    auto set_tima(u8 value) -> void;
    auto set_tma(u8 value) -> void;
    auto set_tac(u8 value) -> void;

private:
    Gameboy &gb;
    u8 div; // DIV: Divider register
    u8 tima; // TIMA: Timer counter
    u8 tma; // TMA: Timer modulo
    u8 tac; // TAC: Timer control

    u32 div_cycles;
    u32 tima_cycles;

    auto is_tima_increment_enabled() -> bool;
    auto get_tima_machine_cycle_frequency() -> u16;
};

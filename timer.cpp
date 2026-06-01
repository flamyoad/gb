//
// Created by Ng Zhen Hao on 30/05/2026.
//

#include "timer.h"
#include "gameboy.h"

#include <utility>

Timer::Timer(Gameboy &gameboy) : gb(gameboy) {
    div = 0;
    tima = 0;
    tma = 0;
    tac = 0;
    div_cycles = 0;
    tima_cycles = 0;
}

auto Timer::tick(u32 m_cycle) -> void {
    div_cycles += m_cycle;
    // This register is incremented at a rate of 16384Hz
    // 4.194304 MHz (Gameboy CPU freq) / 16384 Hz = 256
    while (div_cycles >= 256) {
        div_cycles -= 256;
        div++;
    }

    if (is_tima_increment_enabled()) {
        const auto tima_machine_cycle_frequency = get_tima_machine_cycle_frequency();
        tima_cycles += m_cycle;

        while (tima_cycles >= tima_machine_cycle_frequency) {
            tima_cycles -= tima_machine_cycle_frequency;
            tima++;

            // When TIMA overflows, it is reset to the value in TMA and an interrupt is requested.
            if (tima == 0x00) {
                tima = tma;
                gb.cpu.request_interrupt(Interrupt::Timer);
            }
        }
    }
}

auto Timer::set_div(u8 value) -> void {
    div = 0;
    div_cycles = 0;
}

auto Timer::set_tima(u8 value) -> void {
    tima = value;
}

auto Timer::set_tac(u8 value) -> void {
    tac = value;
}

auto Timer::set_tma(u8 value) -> void {
    tma = value;
}

auto Timer::get_div() -> u8 {
    return div;
}

auto Timer::get_tima() -> u8 {
    return tima;
}

auto Timer::get_tma() -> u8 {
    return tma;
}

auto Timer::get_tac() -> u8 {
    return tac;
}

auto Timer::is_tima_increment_enabled() -> bool {
    return tac >> 2 & 0b1;
}

// https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff07--tac-timer-control
// those are m-cycles, not t-cycles!
auto Timer::get_tima_machine_cycle_frequency() -> u16 {
    switch (tac & 0b11) {
        case 0b00: return 256;
        case 0b01: return 4;
        case 0b10: return 16;
        case 0b11: return 64;
        default: std::unreachable();
    }
}



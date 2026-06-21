//
// Created by Ng Zhen Hao on 20/06/2026.
//

#include <iostream>

#include "serial.h"
#include "gameboy.h"

Serial::Serial(Gameboy &gameboy) : gb(gameboy) {
    sb = 0;
    sc = 0;
}

auto Serial::read() const -> u8 {
    return sb;
}

auto Serial::write(const u8 byte) -> void {
    sb = byte;
}

auto Serial::write_control(const u8 byte) -> void {
    sc = byte;

    // if bit7 enabled
    if (sc >> 7) {
        std::cout << static_cast<char>(sb) << std::flush;

        gb.cpu.request_interrupt(Interrupt::Serial);

        // Bit 7 is cleared when the transfer has finished
        sc &= ~(1 << 7);
    }
}

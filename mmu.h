//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once
#include <array>
#include <vector>

#include "common_types.h"

class Gameboy;

// http://gameboy.mongenel.com/dmg/asmmemmap.html
class Mmu {
public:
    explicit Mmu(Gameboy &gameboy);
    void power_on();
    void load_rom(const std::vector<u8> &rom);
    auto read(u16 address) -> u8;
    void write(u16 address, u8 value);

private:
    Gameboy& gb;

    // 16bit address bus
    std::array<u8, 0x10000> mem {};

    auto bios_check_successful() -> bool;
};

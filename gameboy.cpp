//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "gameboy.h"

#include <fstream>
#include <ios>
#include <iosfwd>
#include <vector>

Gameboy::Gameboy()
    : cpu(*this),
      mmu(*this),
      timer(*this),
      serial(*this) {
}

void Gameboy::load_rom(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM: " + path);
    }

    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto rom = std::vector<u8>(size);
    file.read(reinterpret_cast<char*>(rom.data()), size);

    mmu.load_rom(rom);
}

void Gameboy::tick() {
    const auto m_cycle = cpu.tick();

    if (cpu.state != CpuState::Stopped) {
        timer.tick(m_cycle);
    }
}

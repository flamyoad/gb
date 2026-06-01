//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "gameboy.h"

Gameboy::Gameboy()
    : cpu(*this),
      mmu(*this),
      timer(*this) {
}

void Gameboy::start() {

}

void Gameboy::tick() {
    const auto m_cycle = cpu.tick();

    if (cpu.state != CpuState::Stopped) {
        timer.tick(m_cycle);
    }
}

//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "cpu.h"
#include "mmu.h"
#include "serial.h"
#include "timer.h"

class Gameboy {
public:
    Gameboy();
    void load_rom(const std::string &path);
    void tick();

private:
    Cpu cpu;
    friend class Cpu;

    Mmu mmu;
    friend class Mmu;

    Timer timer;
    friend class Timer;

    Serial serial;
    friend class Serial;
};

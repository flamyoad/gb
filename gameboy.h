//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "cpu.h"
#include "mmu.h"

class Gameboy {
public:
    Gameboy();
    void start();

private:
    Cpu cpu;
    friend class Cpu;

    Mmu mmu;
    friend class Mmu;
};

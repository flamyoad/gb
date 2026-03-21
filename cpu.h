//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "common_types.h"

class Gameboy;

class Cpu {
public:
        explicit Cpu(Gameboy &gb);

        // B, C, D, E, H, L, A, F
        u8 registers[8];

        u16 pc;
        u16 sp;

        //todo: Opcode found in below link. note there are 8bit and 16bit (0xCB)
        // https://meganesu.github.io/generate-gb-opcodes/
};

//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "cpu.h"
#include "gameboy.h"
#include "common_types.h"

Cpu::Cpu(Gameboy &gb) {
    // todo: init register,sp,pc default values
    // https://bgb.bircd.org/pandocs.htm#powerupsequence
    pc = 0x0100;
    sp = 0xFFFE;
}

//
// Created by Ng Zhen Hao on 01/06/2026.
//

#pragma once

#include "../common_types.h"

/*
    Bit 0: VBlank
    Bit 1: LCD STAT
    Bit 2: Timer
    Bit 3: Serial
    Bit 4: Joypad
    Bit 5-7: unused
*/
static constexpr u8 INTERRUPT_MASK = 0x1F;

enum class Interrupt: u8 {
    VBlank = 1 << 0,
    LCD_Stat = 1 << 1,
    Timer = 1 << 2,
    Serial = 1 << 3,
    Joypad = 1 << 4
};
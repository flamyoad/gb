//
// Created by Ng Zhen Hao on 20/06/2026.
//

#pragma once
#include "common_types.h"

class Gameboy;

// https://bgb.bircd.org/pandocs.htm#serialdatatransferlinkcable
class Serial {
public:
    explicit Serial(Gameboy &gameboy);

    auto read() const -> u8;
    auto write(u8 byte) -> void;
    auto write_control(u8 byte) -> void;

private:
    Gameboy &gb;
    u8 sb;
    u8 sc;
};

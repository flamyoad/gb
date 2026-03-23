//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "cpu.h"
#include "gameboy.h"
#include "common_types.h"

Cpu::Cpu(Gameboy &gb) :
    gb(gb) {

    // https://bgb.bircd.org/pandocs.htm#powerupsequence
    pc = 0x0100;
    sp = 0xFFFE;
    a = 0x01; f = 0xB0;
    b = 0x00; c = 0x13;
    d = 0x00; e = 0xD8;
    h = 0x01; l = 0x4D;
}

u16 Cpu::af() const { return static_cast<u16>(a) << 8 | f; }
u16 Cpu::bc() const { return static_cast<u16>(b) << 8 | c; }
u16 Cpu::de() const { return static_cast<u16>(d) << 8 | e; }
u16 Cpu::hl() const { return static_cast<u16>(h) << 8 | l; }

void Cpu::set_af(const u16 value) {
    a = value >> 8;
    f = value & 0xF0; // Bit 0 to 3 must always be zero
}

void Cpu::set_bc(const u16 value) {
    b = value >> 8;
    c = value & 0xFF;
}

void Cpu::set_de(const u16 value) {
    d = value >> 8;
    e = value & 0xFF;
}

void Cpu::set_hl(const u16 value) {
    h = value >> 8;
    l = value & 0xFF;
}

void Cpu::set_flag(const u8 mask, const bool value) {
    if (value) {
        f |= mask;
    } else {
        f &= ~mask;
    }
}

auto Cpu::tick() -> u32 {
    auto opcode = fetch_8bit();
    auto cycle_count = execute(opcode);
    return cycle_count;
}

// https://rgbds.gbdev.io/docs/v0.6.0/gbz80.7#ADC_A,r8
// perhaps we can make this into an abstraction of lets say 0x01 = LD(BC, d16) and 0x02 = LD(BC, Register A)

// https://gekkio.fi/files/gb-docs/gbctr.pdf
// use this for ALU operations
auto Cpu::execute(const u8 opcode) -> u32 {
    switch (opcode) {
        case 0x00:
            return 1;

        case 0x01:
            set_bc(fetch_16bit());
            return 3;

        case 0x02:
            write_mmu(bc(), a);
            return 2;

        case 0x03:
            set_bc(bc() + 1);
            return 2;

        case 0x04: {
            // todo : extract this perhaps aftet few more opcodes
            const u8 result = b + 1;
            set_flag(FLAG_Z, result == 0);
            set_flag(FLAG_N, false);
            set_flag(FLAG_H, (b & 0xF) == 0xF); // Set to true if bit3 overflowed into bit4
            b = result;
            return 1;
        }

        case 0xCB:
            return execute_cb_opcode(opcode);
    }
}

auto Cpu::execute_cb_opcode(u8 opcode) -> u32 {

}

auto Cpu::fetch_8bit() -> u8 {
    auto byte = read_mmu(pc);
    ++pc;
    return byte;
}

auto Cpu::fetch_16bit() -> u16 {
    auto lower_byte = fetch_8bit();
    auto higher_byte = fetch_8bit();
    return static_cast<u16>(higher_byte) << 8 | lower_byte;
}

auto Cpu::read_mmu(u16 address) -> u8 {
    return gb.mmu.read(address);
}

auto Cpu::write_mmu(u16 address, u8 value) -> void {
    gb.mmu.write(address, value);
}

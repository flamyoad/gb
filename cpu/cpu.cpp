//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "cpu.h"
#include "../gameboy.h"
#include "../common_types.h"

Cpu::Cpu(Gameboy &gb) :
    gb(gb),
    af(a, f),
    bc(b, c),
    de(d, e),
    hl(h, l) {

    // https://bgb.bircd.org/pandocs.htm#powerupsequence
    pc = 0x0100;
    sp = 0xFFFE;
    a.value = 0x01;
    f.value = 0xB0;
    b.value = 0x00;
    c.value = 0x13;
    d.value = 0x00;
    e.value = 0xD8;
    h.value = 0x01;
    l.value = 0x4D;
}

auto Cpu::tick() -> u32 {
    auto opcode = fetch_8bit();
    auto cycle_count = execute(opcode);
    return cycle_count;
}

auto Cpu::set_flag(const u8 mask, const bool value) -> void {
    f.set(mask, value);
}

// https://rgbds.gbdev.io/docs/v0.6.0/gbz80.7#ADC_A,r8
// perhaps we can make this into an abstraction of lets say 0x01 = LD(BC, d16) and 0x02 = LD(BC, Register A)

// https://gekkio.fi/files/gb-docs/gbctr.pdf
// use this for ALU operations
auto Cpu::execute(const u8 opcode) -> u32 {
    switch (opcode) {
        case 0x00: return NOP();
        case 0x01: return LD_r16_n16(bc, fetch_16bit());
        case 0x02: return LD_m16_r8(bc, a);
        case 0x03: return INC_r16(bc);
        case 0x04: return INC_r8(b);
        case 0x05: return DEC_r8(b);
        case 0x06: return LD_r8_n8(b);
        case 0x07: return RLCA();
        case 0x08: return LD_n16_SP();
        case 0x09: return ADD_HL_r16(bc);
        case 0x0A: return LD_r8_r16(a, bc);
        case 0x0B: return DEC_r16(bc);

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

auto Cpu::NOP() -> u8 {
    return 1;
}

auto Cpu::INC_r8(Register &reg) -> u8 {
    const u8 result = reg.value + 1;
    set_flag(FLAG_Z, result == 0);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, (reg.value & 0xF) == 0xF); // Set to true if bit3 overflowed into bit4
    reg.value = result;
    return 1;
}

auto Cpu::INC_r16(RegisterPair &reg_pair) -> u8 {
    reg_pair.set(reg_pair.value() + 1);
    return 2;
}

auto Cpu::DEC_r8(Register &reg) -> u8 {
    const u8 result = reg.value - 1;
    set_flag(FLAG_Z, result == 0);
    set_flag(FLAG_N, true);
    set_flag(FLAG_H, (reg.value & 0xF) == 0x0); // Set if borrow from bit 4
    reg.value = result;
    return 1;
}

auto Cpu::DEC_r16(RegisterPair &reg_pair) -> u8 {
    reg_pair.set(reg_pair.value() - 1);
    return 2;
}

auto Cpu::LD_r8_n8(Register &reg) -> u8 {
    reg.value = fetch_8bit();
    return 2;
}

auto Cpu::LD_r16_n16(RegisterPair &reg_pair, u16 value) -> u8 {
    reg_pair.set(value);
    return 3;
}

auto Cpu::LD_m16_r8(RegisterPair &reg_pair, Register reg) -> u8 {
    write_mmu(reg_pair.value(), reg.value);
    return 2;
}

auto Cpu::LD_r8_r16(Register &reg, RegisterPair reg_pair) -> u8 {
    reg.value = read_mmu(reg_pair.value());
    return 2;
}

auto Cpu::LD_n16_SP() -> u8 {
    const u16 n16 = fetch_16bit();
    const u8 sp_lsb = sp & 0xFF;
    const u8 sp_msb = (sp >> 8) & 0xFF;
    write_mmu(n16, sp_lsb);
    write_mmu(n16 + 1, sp_msb);
    return 5;
}

auto Cpu::RLCA() -> u8 {
    const u8 reg_a_msb = (a.value >> 7 & 0x1);
    a.value = a.value << 1 | a.value >> 7;
    set_flag(FLAG_Z, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_C, reg_a_msb);
    return 1;
}

auto Cpu::ADD_HL_r16(const RegisterPair reg_pair) -> u8 {
    // is overflow from bit 11?
    const bool flag_h = (hl.value() & 0xFFF) + (reg_pair.value() & 0xFFF) > 0xFFF;

    // is overflow from bit 15?
    const bool flag_c = static_cast<u32>(hl.value()) + reg_pair.value() > 0xFFFF;

    hl.set(hl.value() + reg_pair.value());
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, flag_h);
    set_flag(FLAG_C, flag_c);
    return 2;
}


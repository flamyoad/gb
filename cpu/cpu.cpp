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
    auto opcode = fetch_unsigned_8bit();
    auto cycle_count = execute(opcode);
    return cycle_count;
}

auto Cpu::get_flag_value(Flag flag) -> u8 {
    return f.value & static_cast<u8>(flag) ? 1 : 0;

}

auto Cpu::set_flag_value(Flag flag, const bool value) -> void {
    f.set(static_cast<u8>(flag), value);
}

// https://rgbds.gbdev.io/docs/v0.6.0/gbz80.7#ADC_A,r8
// perhaps we can make this into an abstraction of lets say 0x01 = LD(BC, d16) and 0x02 = LD(BC, Register A)

// https://gekkio.fi/files/gb-docs/gbctr.pdf
// use this for ALU operations
auto Cpu::execute(const u8 opcode) -> u32 {
    switch (opcode) {
        case 0x00: return NOP();
        case 0x01: return LD_r16_n16(bc);
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
        case 0x0C: return INC_r8(c);
        case 0x0D: return DEC_r8(c);
        case 0x0E: return LD_r8_n8(c);
        case 0x0F: return RRCA();
        case 0x10: return STOP();
        case 0x11: return LD_r16_n16(de);
        case 0x12: return LD_m16_r8(de, a);
        case 0x13: return INC_r16(de);
        case 0x14: return INC_r8(d);
        case 0x15: return DEC_r8(d);
        case 0x16: return LD_r8_n8(d);
        case 0x17: return RLA();
        case 0x18: return JR_s8();
        case 0x19: return ADD_HL_r16(de);
        case 0x1A: return LD_r8_n8(a);
        case 0x1B: return DEC_r16(de);
        case 0x1C: return INC_r8(e);
        case 0x1D: return DEC_r8(e);
        case 0x1E: return LD_r8_n8(e);
        case 0x1F: return RRA();
        case 0x20: return JR_cc_s8(JumpCondition(Flag::Z, Condition::isZero));
        case 0x21: return LD_r16_n16(de);
        case 0x22: return LD_HLinc_A();
        case 0x23: return INC_r16(hl);
        case 0x24: return INC_r8(h);
        case 0x25: return DEC_r8(h);
        case 0x26: return LD_r8_n8(h);
        case 0x27: return DAA();
        case 0x28: return JR_cc_s8(JumpCondition(Flag::Z, Condition::isOne));
        case 0x29: return ADD_HL_r16(hl);
        case 0x2A: return LD_A_HLinc();
        case 0x2B: return DEC_r16(hl);
        case 0x2C: return INC_r8(l);
        case 0x2D: return DEC_r8(l);
        case 0x2E: return LD_r8_n8(e);
        case 0x2F: return CPL();
        case 0x30: return JR_cc_s8(JumpCondition(Flag::C, Condition::isZero));
        case 0x31: return LD_SP_n16();
        case 0x32: return LD_HLdec_A();
        case 0x33: return INC_SP();
        case 0x34: return INC_r16(hl);
        case 0x35: return DEC_r16(hl);
        case 0x36: return LD_m16_n8(hl);
        case 0x37: return SCF();
        case 0x38: return JR_cc_s8(JumpCondition(Flag::C, Condition::isOne));
        case 0x39: return ADD_HL_SP();
        case 0x3A: return LD_A_HLdec();
        case 0x3B: return DEC_SP();
        case 0x3C: return INC_r8(a);
        case 0x3D: return DEC_r8(a);
        case 0x3E: return LD_r8_n8(l);
        case 0x3F: return CCF();
        case 0x40: return LD_r8_r8(b, b);
        case 0x41: return LD_r8_r8(b, c);
        case 0x42: return LD_r8_r8(b, d);
        case 0x43: return LD_r8_r8(b, e);
        case 0x44: return LD_r8_r8(b, h);
        case 0x45: return LD_r8_r8(b, l);
        case 0x46: return LD_r8_m16(b, hl);
        case 0x47: return LD_r8_r8(b, a);
        case 0x48: return LD_r8_r8(c, b);
        case 0x49: return LD_r8_r8(c ,c);
        case 0x4A: return LD_r8_r8(c, d);
        case 0x4B: return LD_r8_r8(c, e);
        case 0x4C: return LD_r8_r8(c, h);
        case 0x4D: return LD_r8_r8(c, l);
        case 0x4E: return LD_r8_m16(c, hl);
        case 0x4F: return LD_r8_r8(c, a);
        case 0x50: return LD_r8_r8(d, b);
        case 0x51: return LD_r8_r8(d, c);
        case 0x52: return LD_r8_r8(d, d);
        case 0x53: return LD_r8_r8(d, e);
        case 0x54: return LD_r8_r8(d, h);
        case 0x55: return LD_r8_r8(d, l);
        case 0x56: return LD_r8_m16(d, hl);
        case 0x57: return LD_r8_r8(d, a);
        case 0x58: return LD_r8_r8(e, b);
        case 0x59: return LD_r8_r8(e, c);
        case 0x5A: return LD_r8_r8(e, d);
        case 0x5B: return LD_r8_r8(e, e);
        case 0x5C: return LD_r8_r8(e, h);
        case 0x5D: return LD_r8_r8(e, l);
        case 0x5E: return LD_r8_m16(e, hl);
        case 0x5F: return LD_r8_r8(e, a);
        case 0x60: return LD_r8_r8(d, b);
        case 0x61: return LD_r8_r8(h, c);
        case 0x62: return LD_r8_r8(h, d);
        case 0x63: return LD_r8_r8(h, e);
        case 0x64: return LD_r8_r8(h, h);
        case 0x65: return LD_r8_r8(h, l);
        case 0x66: return LD_r8_m16(h, hl);
        case 0x67: return LD_r8_r8(h, a);
        case 0x68: return LD_r8_r8(l, b);
        case 0x69: return LD_r8_r8(l, c);
        case 0x6A: return LD_r8_r8(l, d);
        case 0x6B: return LD_r8_r8(l, e);
        case 0x6C: return LD_r8_r8(l, h);
        case 0x6D: return LD_r8_r8(l, l);
        case 0x6E: return LD_r8_m16(l, hl);
        case 0x6F: return LD_r8_r8(l, a);
        case 0x70: return LD_m16_r8(hl, b);
        case 0x71: return LD_m16_r8(hl, c);
        case 0x72: return LD_m16_r8(hl, d);
        case 0x73: return LD_m16_r8(hl, e);
        case 0x74: return LD_m16_r8(hl, h);
        case 0x75: return LD_m16_r8(hl, l);
        case 0x76: return HALT();
        case 0x77: return LD_m16_r8(hl, a);
        case 0x78: return LD_r8_r8(a, b);
        case 0x79: return LD_r8_r8(a, c);
        case 0x7A: return LD_r8_r8(a, d);
        case 0x7B: return LD_r8_r8(a, e);
        case 0x7C: return LD_r8_r8(a, h);
        case 0x7D: return LD_r8_r8(a, l);
        case 0x7E: return LD_r8_m16(a, hl);
        case 0x7F: return LD_r8_r8(a, a);

        case 0xCB:
            return execute_cb_opcode(opcode);
    }
}

auto Cpu::execute_cb_opcode(u8 opcode) -> u32 {

}

auto Cpu::fetch_unsigned_8bit() -> u8 {
    auto byte = read_mmu(pc);
    ++pc;
    return byte;
}

auto Cpu::fetch_unsigned_16bit() -> u16 {
    auto lower_byte = fetch_unsigned_8bit();
    auto higher_byte = fetch_unsigned_8bit();
    return static_cast<u16>(higher_byte) << 8 | lower_byte;
}

auto Cpu::fetch_signed_8bit() -> i8 {
    const u8 unsigned_byte = fetch_unsigned_8bit();
    return static_cast<i8>(unsigned_byte);
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

auto Cpu::STOP() -> u8 {
    // todo: what do here boi!?
    // Execution of a STOP instruction stops both the system clock and oscillator circuit. STOP mode is entered and the LCD controller also stops.
    fetch_unsigned_8bit(); // Discard the second 0x00 byte here. because STOP opcode is 0x1000
    return 1;
}

// Unconditional jump to the relative address specified by the signed 8-bit operand
auto Cpu::JR_s8() -> u8 {
    const auto s8 = fetch_signed_8bit();
    pc = pc + s8;
    return 3;
}

// If flag is 0 or 1 (depends on condition), jump s8 steps from the current address stored in the program counter (PC).
// If not, the instruction following the current JP instruction is executed (as usual).
auto Cpu::JR_cc_s8(JumpCondition cc) -> u8 {
    if (cc.is_valid(f)) {
        return JR_s8();
    }

    fetch_unsigned_8bit();
    return 2;
}

auto Cpu::INC_r8(Register &reg) -> u8 {
    const u8 result = reg.value + 1;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, (reg.value & 0xF) == 0xF); // Set to true if bit3 overflowed into bit4
    reg.value = result;
    return 1;
}

auto Cpu::INC_r16(RegisterPair &reg_pair) -> u8 {
    reg_pair.set(reg_pair.value() + 1);
    return 2;
}

auto Cpu::INC_SP() -> u8 {
    sp += 1;
    return 2;
}

auto Cpu::DEC_r8(Register &reg) -> u8 {
    const u8 result = reg.value - 1;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, (reg.value & 0xF) == 0x0); // Set if borrow from bit 4
    reg.value = result;
    return 1;
}

auto Cpu::DEC_r16(RegisterPair &reg_pair) -> u8 {
    reg_pair.set(reg_pair.value() - 1);
    return 2;
}

auto Cpu::DEC_SP() -> u8 {
    sp -= 1;
    return 2;
}

auto Cpu::LD_r8_n8(Register &reg) -> u8 {
    reg.value = fetch_unsigned_8bit();
    return 2;
}

auto Cpu::LD_r8_r8(Register &reg_into, Register reg_from)-> u8 {
    reg_into.value = reg_from.value;
    return 1;
}

auto Cpu::LD_r8_m16(Register &reg_into, RegisterPair reg_pair) -> u8 {
    reg_into.value = read_mmu(reg_pair.value());
    return 2;
}

auto Cpu::LD_m16_n8(RegisterPair &reg_pair) -> u8 {
    write_mmu(reg_pair.value(), fetch_unsigned_8bit());
    return 3;
}

auto Cpu::LD_r16_n16(RegisterPair &reg_pair) -> u8 {
    const u16 value = fetch_unsigned_16bit();
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
    const u16 n16 = fetch_unsigned_16bit();
    const u8 sp_lsb = sp & 0xFF;
    const u8 sp_msb = (sp >> 8) & 0xFF;
    write_mmu(n16, sp_lsb);
    write_mmu(n16 + 1, sp_msb);
    return 5;
}

auto Cpu::LD_SP_n16() -> u8 {
    sp = fetch_unsigned_16bit();
    return 3;
}

auto Cpu::ADD_r8_r8(Register &reg_into, Register reg_from) -> u8 {
    // todo:
    return 1;
}

auto Cpu::ADD_HL_r16(const RegisterPair reg_pair) -> u8 {
    // is overflow from bit 11?
    const bool flag_h = (hl.value() & 0xFFF) + (reg_pair.value() & 0xFFF) > 0xFFF;

    // is overflow from bit 15?
    const bool flag_c = static_cast<u32>(hl.value()) + reg_pair.value() > 0xFFFF;

    hl.set(hl.value() + reg_pair.value());
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::ADD_HL_SP() -> u8 {
    // is overflow from bit 11?
    const bool flag_h = (hl.value() & 0xFFF) + (sp & 0xFFF) > 0xFFF;

    // is overflow from bit 15?
    const bool flag_c = static_cast<u32>(hl.value()) + sp > 0xFFFF;

    hl.set(hl.value() + sp);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::RLCA() -> u8 {
    const u8 reg_a_msb = (a.value >> 7 & 0x1);
    a.value = a.value << 1 | a.value >> 7;
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, reg_a_msb);
    return 1;
}

// Rotate right circular (Accumulator)
auto Cpu::RRCA() -> u8 {
    const u8 reg_a_lsb = a.value & 0x1;
    a.value = a.value >> 1 | a.value << 7;
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, reg_a_lsb);
    return 1;
}

// Rotate left (Accumulator)
auto Cpu::RLA() -> u8 {
    const u8 reg_a_msb = a.value >> 7 & 0x1;
    a.value = a.value << 1 | get_flag_value(Flag::C);
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, reg_a_msb);
    return 1;
}

// Rotate right (Accumulator)
auto Cpu::RRA() -> u8 {
    const u8 reg_a_lsb = a.value & 0x1;
    a.value = a.value >> 1 | get_flag_value(Flag::C) << 7;
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, reg_a_lsb);
    return 1;
}

// Load to the 8-bit A register, data from the absolute address specified by the 16-bit register HL.
// The value of HL is decremented after the memory read.
auto Cpu::LD_A_HLdec() -> u8 {
    a.value = read_mmu(hl.value());
    hl.set(hl.value() - 1);
    return 2;
}

// Load to the absolute address specified by the 16-bit register HL, data from the 8-bit A register.
// The value of HL is decremented after the memory write
auto Cpu::LD_HLdec_A() -> u8 {
    write_mmu(hl.value(), a.value);
    hl.set(hl.value() - 1);
    return 2;
}

// Load to the 8-bit A register, data from the absolute address specified by the 16-bit register HL.
// The value of HL is incremented after the memory read.
auto Cpu::LD_A_HLinc() -> u8 {
    a.value = read_mmu(hl.value());
    hl.set(hl.value() + 1);
    return 2;
}

// Load to the absolute address specified by the 16-bit register HL, data from the 8-bit A register.
// The value of HL is incremented after the memory write
auto Cpu::LD_HLinc_A() -> u8 {
    a.value = read_mmu(hl.value());
    hl.set(hl.value() + 1);
    return 2;
}

// Adjust the accumulator (register A) to a binary-coded decimal (BCD) number after BCD addition and subtraction operations.
// Example is : 70(dec) converted to 0x70(BCD) instead of its actual value 0x46(hex)
auto Cpu::DAA() -> u8 {
    u8 offset = 0;

    // The bitwise OR operator is equivalent to addition if the two values share no bits in common (mutually exclusive)
    if (!get_flag_value(Flag::N)) { // Addition
        if (get_flag_value(Flag::H) || (a.value & 0x0F) > 0x09) {
            offset |= 0x06;
        }
        if (get_flag_value(Flag::C) || a.value > 0x99) {
            offset |= 0x60;
            set_flag_value(Flag::C, true);
        }
        a.value += offset;
    } else { // Subtraction
        if (get_flag_value(Flag::H)) {
            offset |= 0x06;
        }
        if (get_flag_value(Flag::C)) {
            offset |= 0x60;
        }
        a.value -= offset;
    }
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::H, false);
    return 1;
}

auto Cpu::CPL() -> u8 {
    a.value = ~a.value;
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, true);
    return 1;
}

// Complement carry flag
auto Cpu::CCF() -> u8 {
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, !get_flag_value(Flag::C));
    return 1;
}

auto Cpu::SCF() -> u8 {
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, true);
    return 1;
}

auto Cpu::HALT() -> u8 {
    ime = 0;
    return 1;
}

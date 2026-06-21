//
// Created by Ng Zhen Hao on 21/03/2026.
//
#include "cpu.h"

#include <cassert>
#include <format>

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

    ime = false;
    interrupt_flag = 0xE1;
    interrupt_enable = 0;
    state = CpuState::Running;
    ime_next = false;
}

auto Cpu::tick() -> u32 {
    // https://gbdev.io/pandocs/halt.html
    // The CPU wakes up as soon as an interrupt is pending, that is, when the bitwise AND of IE and IF is non-zero.
    if ((interrupt_enable & interrupt_flag) != 0) {
        if (state == CpuState::Halted) {
            state = CpuState::Running;
        }

        if (state == CpuState::Stopped) {
            // STOP is terminated by one of the P10 to P13 lines going low (Joypad button is pressed)
            if (interrupt_flag & static_cast<u8>(Interrupt::Joypad)) {
                state = CpuState::Running;
            }
        }
    }

    if (ime && get_pending_interrupt() != 0) {
        return handle_interrupt();
    }

    if (state == CpuState::Halted || state == CpuState::Stopped) {
        return 1;
    }

    auto opcode = fetch_unsigned_8bit();
    auto cycle_count = execute(opcode);

    if (ime_next) {
        ime_next = false;
        ime = true;
    }

    return cycle_count;
}

auto Cpu::request_interrupt(Interrupt interrupt) -> void {
    interrupt_flag |= static_cast<u8>(interrupt);
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
        case 0x1A: return LD_r8_r16(a, de);
        case 0x1B: return DEC_r16(de);
        case 0x1C: return INC_r8(e);
        case 0x1D: return DEC_r8(e);
        case 0x1E: return LD_r8_n8(e);
        case 0x1F: return RRA();
        case 0x20: return JR_cc_s8(FlagCondition(Flag::Z, Condition::isNotSet));
        case 0x21: return LD_r16_n16(hl);
        case 0x22: return LD_HLinc_A();
        case 0x23: return INC_r16(hl);
        case 0x24: return INC_r8(h);
        case 0x25: return DEC_r8(h);
        case 0x26: return LD_r8_n8(h);
        case 0x27: return DAA();
        case 0x28: return JR_cc_s8(FlagCondition(Flag::Z, Condition::isSet));
        case 0x29: return ADD_HL_r16(hl);
        case 0x2A: return LD_A_HLinc();
        case 0x2B: return DEC_r16(hl);
        case 0x2C: return INC_r8(l);
        case 0x2D: return DEC_r8(l);
        case 0x2E: return LD_r8_n8(l);
        case 0x2F: return CPL();
        case 0x30: return JR_cc_s8(FlagCondition(Flag::C, Condition::isNotSet));
        case 0x31: return LD_SP_n16();
        case 0x32: return LD_HLdec_A();
        case 0x33: return INC_SP();
        case 0x34: return INC_m16(hl);
        case 0x35: return DEC_m16(hl);
        case 0x36: return LD_m16_n8(hl);
        case 0x37: return SCF();
        case 0x38: return JR_cc_s8(FlagCondition(Flag::C, Condition::isSet));
        case 0x39: return ADD_HL_SP();
        case 0x3A: return LD_A_HLdec();
        case 0x3B: return DEC_SP();
        case 0x3C: return INC_r8(a);
        case 0x3D: return DEC_r8(a);
        case 0x3E: return LD_r8_n8(a);
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
        case 0x60: return LD_r8_r8(h, b);
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
        case 0x80: return ADD_r8_r8(a, b);
        case 0x81: return ADD_r8_r8(a, c);
        case 0x82: return ADD_r8_r8(a, d);
        case 0x83: return ADD_r8_r8(a, e);
        case 0x84: return ADD_r8_r8(a, h);
        case 0x85: return ADD_r8_r8(a, l);
        case 0x86: return ADD_r8_m16(a, hl);
        case 0x87: return ADD_r8_r8(a, a);
        case 0x88: return ADC_r8_r8(a, b);
        case 0x89: return ADC_r8_r8(a, c);
        case 0x8A: return ADC_r8_r8(a, d);
        case 0x8B: return ADC_r8_r8(a, e);
        case 0x8C: return ADC_r8_r8(a, h);
        case 0x8D: return ADC_r8_r8(a, l);
        case 0x8E: return ADC_r8_m16(a, hl);
        case 0x8F: return ADC_r8_r8(a, a);
        case 0x90: return SUB_r8(b);
        case 0x91: return SUB_r8(c);
        case 0x92: return SUB_r8(d);
        case 0x93: return SUB_r8(e);
        case 0x94: return SUB_r8(h);
        case 0x95: return SUB_r8(l);
        case 0x96: return SUB_m16(hl);
        case 0x97: return SUB_r8(a);
        case 0x98: return SBC_r8(b);
        case 0x99: return SBC_r8(c);
        case 0x9A: return SBC_r8(d);
        case 0x9B: return SBC_r8(e);
        case 0x9C: return SBC_r8(h);
        case 0x9D: return SBC_r8(l);
        case 0x9E: return SBC_m16(hl);
        case 0x9F: return SBC_r8(a);
        case 0xA0: return AND_r8(b);
        case 0xA2: return AND_r8(d);
        case 0xA3: return AND_r8(e);
        case 0xA4: return AND_r8(h);
        case 0xA5: return AND_r8(l);
        case 0xA1: return AND_r8(c);
        case 0xA6: return AND_m16(hl);
        case 0xA7: return AND_r8(a);
        case 0xA8: return XOR_r8(b);
        case 0xA9: return XOR_r8(c);
        case 0xAA: return XOR_r8(d);
        case 0xAB: return XOR_r8(e);
        case 0xAC: return XOR_r8(h);
        case 0xAD: return XOR_r8(l);
        case 0xAE: return XOR_m16(hl);
        case 0xAF: return XOR_r8(a);
        case 0xB0: return OR_r8(b);
        case 0xB1: return OR_r8(c);
        case 0xB2: return OR_r8(d);
        case 0xB3: return OR_r8(e);
        case 0xB4: return OR_r8(h);
        case 0xB5: return OR_r8(l);
        case 0xB6: return OR_m16(hl);
        case 0xB7: return OR_r8(a);
        case 0xB8: return CP_r8(b);
        case 0xB9: return CP_r8(c);
        case 0xBA: return CP_r8(d);
        case 0xBB: return CP_r8(e);
        case 0xBC: return CP_r8(h);
        case 0xBD: return CP_r8(l);
        case 0xBE: return CP_m16(hl);
        case 0xBF: return CP_r8(a);
        case 0xC0: return RET(FlagCondition(Flag::Z, Condition::isNotSet));
        case 0xC1: return POP(bc);
        case 0xC2: return JP_cc_n16(FlagCondition(Flag::Z, Condition::isNotSet));
        case 0xC3: return JP_n16();
        case 0xC4: return CALL_cc(FlagCondition(Flag::Z, Condition::isNotSet));
        case 0xC5: return PUSH(bc);
        case 0xC6: return ADD_r8_m8(a);
        case 0xC7: return RST(0);
        case 0xC8: return RET(FlagCondition(Flag::Z, Condition::isSet));
        case 0xC9: return RET();
        case 0xCA: return JP_cc_n16(FlagCondition(Flag::Z, Condition::isSet));
        case 0xCB: return execute_cb_opcode(fetch_unsigned_8bit());
        case 0xCC: return CALL_cc(FlagCondition(Flag::Z, Condition::isSet));
        case 0xCD: return CALL();
        case 0xCE: return ADC_r8_n8(a);
        case 0xCF: return RST(1);
        case 0xD0: return RET(FlagCondition(Flag::C, Condition::isNotSet));
        case 0xD1: return POP(de);
        case 0xD2: return JP_cc_n16(FlagCondition(Flag::C, Condition::isNotSet));
        case 0xD3: // no-op
        case 0xD4: return CALL_cc(FlagCondition(Flag::C, Condition::isNotSet));
        case 0xD5: return PUSH(de);
        case 0xD6: return SUB_n8();
        case 0xD7: return RST(2);
        case 0xD8: return RET(FlagCondition(Flag::C, Condition::isSet));
        case 0xD9: return RETI();
        case 0xDA: return JP_cc_n16(FlagCondition(Flag::C, Condition::isSet));
        case 0xDB: // no-op
        case 0xDC: return CALL_cc(FlagCondition(Flag::C, Condition::isSet));
        case 0xDD: // no-op
        case 0xDE: return SBC_n8();
        case 0xDF: return RST(3);
        case 0xE0: return LDH_a8_r8(a);
        case 0xE1: return POP(hl);
        case 0xE2: return LDH_m8_r8(c, a);
        case 0xE3: // no-op
        case 0xE4: // no-op
        case 0xE5: return PUSH(hl);
        case 0xE6: return AND_n8();
        case 0xE7: return RST(4);
        case 0xE8: return ADD_SP_s8();
        case 0xE9: return JP_r16(hl);
        case 0xEA: return LD_a16_r8(a);
        case 0xEB: // no-op
        case 0xEC: // no-op
        case 0xED: // no-op
        case 0xEE: return XOR_n8();
        case 0xEF: return RST(5);
        case 0xF0: return LDH_r8_a8(a);
        case 0xF1: return POP(af);
        case 0xF2: return LDH_r8_m8(a, c);
        case 0xF3: return DI();
        case 0xF4: // no-op
        case 0xF5: return PUSH(af);
        case 0xF6: return OR_n8();
        case 0xF7: return RST(6);
        case 0xF8: return LD_HL_SP_s8();
        case 0xF9: return LD_SP_r16(hl);
        case 0xFA: return LD_r8_a16(a);
        case 0xFB: return EI();
        case 0xFC: // no-op
        case 0xFD: // no-op
        case 0xFE: return CP_n8();
        case 0xFF: return RST(7);
        default:
            throw std::runtime_error(
                std::format("Illegal opcode: 0x{:02X} at PC: 0x{:04X}", opcode, pc - 1)
            );
    }
}

auto Cpu::execute_cb_opcode(u8 opcode) -> u32 {
    switch (opcode) {
        case 0x00: return RLC(b);
        case 0x01: return RLC(c);
        case 0x02: return RLC(d);
        case 0x03: return RLC(e);
        case 0x04: return RLC(h);
        case 0x05: return RLC(l);
        case 0x06: return RLC(hl);
        case 0x07: return RLC(a);
        case 0x08: return RRC(b);
        case 0x09: return RRC(c);
        case 0x0A: return RRC(d);
        case 0x0B: return RRC(e);
        case 0x0C: return RRC(h);
        case 0x0D: return RRC(l);
        case 0x0E: return RRC(hl);
        case 0x0F: return RRC(a);
        case 0x10: return RL(b);
        case 0x11: return RL(c);
        case 0x12: return RL(d);
        case 0x13: return RL(e);
        case 0x14: return RL(h);
        case 0x15: return RL(l);
        case 0x16: return RL(hl);
        case 0x17: return RL(a);
        case 0x18: return RR(b);
        case 0x19: return RR(c);
        case 0x1A: return RR(d);
        case 0x1B: return RR(e);
        case 0x1C: return RR(h);
        case 0x1D: return RR(l);
        case 0x1E: return RR(hl);
        case 0x1F: return RR(a);
        case 0x20: return SLA(b);
        case 0x21: return SLA(c);
        case 0x22: return SLA(d);
        case 0x23: return SLA(e);
        case 0x24: return SLA(h);
        case 0x25: return SLA(l);
        case 0x26: return SLA(hl);
        case 0x27: return SLA(a);
        case 0x28: return SRA(b);
        case 0x29: return SRA(c);
        case 0x2A: return SRA(d);
        case 0x2B: return SRA(e);
        case 0x2C: return SRA(h);
        case 0x2D: return SRA(l);
        case 0x2E: return SRA(hl);
        case 0x2F: return SRA(a);
        case 0x30: return SWAP(b);
        case 0x31: return SWAP(c);
        case 0x32: return SWAP(d);
        case 0x33: return SWAP(e);
        case 0x34: return SWAP(h);
        case 0x35: return SWAP(l);
        case 0x36: return SWAP(hl);
        case 0x37: return SWAP(a);
        case 0x38: return SRL(b);
        case 0x39: return SRL(c);
        case 0x3A: return SRL(d);
        case 0x3B: return SRL(e);
        case 0x3C: return SRL(h);
        case 0x3D: return SRL(l);
        case 0x3E: return SRL(hl);
        case 0x3F: return SRL(a);
        case 0x40: return BIT(0, b);
        case 0x41: return BIT(0, c);
        case 0x42: return BIT(0, d);
        case 0x43: return BIT(0, e);
        case 0x44: return BIT(0, h);
        case 0x45: return BIT(0, l);
        case 0x46: return BIT(0, hl);
        case 0x47: return BIT(0, a);
        case 0x48: return BIT(1, b);
        case 0x49: return BIT(1, c);
        case 0x4A: return BIT(1, d);
        case 0x4B: return BIT(1, e);
        case 0x4C: return BIT(1, h);
        case 0x4D: return BIT(1, l);
        case 0x4E: return BIT(1, hl);
        case 0x4F: return BIT(1, a);
        case 0x50: return BIT(2, b);
        case 0x51: return BIT(2, c);
        case 0x52: return BIT(2, d);
        case 0x53: return BIT(2, e);
        case 0x54: return BIT(2, h);
        case 0x55: return BIT(2, l);
        case 0x56: return BIT(2, hl);
        case 0x57: return BIT(2, a);
        case 0x58: return BIT(3, b);
        case 0x59: return BIT(3, c);
        case 0x5A: return BIT(3, d);
        case 0x5B: return BIT(3, e);
        case 0x5C: return BIT(3, h);
        case 0x5D: return BIT(3, l);
        case 0x5E: return BIT(3, hl);
        case 0x5F: return BIT(3, a);
        case 0x60: return BIT(4, b);
        case 0x61: return BIT(4, c);
        case 0x62: return BIT(4, d);
        case 0x63: return BIT(4, e);
        case 0x64: return BIT(4, h);
        case 0x65: return BIT(4, l);
        case 0x66: return BIT(4, hl);
        case 0x67: return BIT(4, a);
        case 0x68: return BIT(5, b);
        case 0x69: return BIT(5, c);
        case 0x6A: return BIT(5, d);
        case 0x6B: return BIT(5, e);
        case 0x6C: return BIT(5, h);
        case 0x6D: return BIT(5, l);
        case 0x6E: return BIT(5, hl);
        case 0x6F: return BIT(5, a);
        case 0x70: return BIT(6, b);
        case 0x71: return BIT(6, c);
        case 0x72: return BIT(6, d);
        case 0x73: return BIT(6, e);
        case 0x74: return BIT(6, h);
        case 0x75: return BIT(6, l);
        case 0x76: return BIT(6, hl);
        case 0x77: return BIT(6, a);
        case 0x78: return BIT(7, b);
        case 0x79: return BIT(7, c);
        case 0x7A: return BIT(7, d);
        case 0x7B: return BIT(7, e);
        case 0x7C: return BIT(7, h);
        case 0x7D: return BIT(7, l);
        case 0x7E: return BIT(7, hl);
        case 0x7F: return BIT(7, a);
        case 0x80: return RES(0, b);
        case 0x81: return RES(0, c);
        case 0x82: return RES(0, d);
        case 0x83: return RES(0, e);
        case 0x84: return RES(0, h);
        case 0x85: return RES(0, l);
        case 0x86: return RES(0, hl);
        case 0x87: return RES(0, a);
        case 0x88: return RES(1, b);
        case 0x89: return RES(1, c);
        case 0x8A: return RES(1, d);
        case 0x8B: return RES(1, e);
        case 0x8C: return RES(1, h);
        case 0x8D: return RES(1, l);
        case 0x8E: return RES(1, hl);
        case 0x8F: return RES(1, a);
        case 0x90: return RES(2, b);
        case 0x91: return RES(2, c);
        case 0x92: return RES(2, d);
        case 0x93: return RES(2, e);
        case 0x94: return RES(2, h);
        case 0x95: return RES(2, l);
        case 0x96: return RES(2, hl);
        case 0x97: return RES(2, a);
        case 0x98: return RES(3, b);
        case 0x99: return RES(3, c);
        case 0x9A: return RES(3, d);
        case 0x9B: return RES(3, e);
        case 0x9C: return RES(3, h);
        case 0x9D: return RES(3, l);
        case 0x9E: return RES(3, hl);
        case 0x9F: return RES(3, a);
        case 0xA0: return RES(4, b);
        case 0xA1: return RES(4, c);
        case 0xA2: return RES(4, d);
        case 0xA3: return RES(4, e);
        case 0xA4: return RES(4, h);
        case 0xA5: return RES(4, l);
        case 0xA6: return RES(4, hl);
        case 0xA7: return RES(4, a);
        case 0xA8: return RES(5, b);
        case 0xA9: return RES(5, c);
        case 0xAA: return RES(5, d);
        case 0xAB: return RES(5, e);
        case 0xAC: return RES(5, h);
        case 0xAD: return RES(5, l);
        case 0xAE: return RES(5, hl);
        case 0xAF: return RES(5, a);
        case 0xB0: return RES(6, b);
        case 0xB1: return RES(6, c);
        case 0xB2: return RES(6, d);
        case 0xB3: return RES(6, e);
        case 0xB4: return RES(6, h);
        case 0xB5: return RES(6, l);
        case 0xB6: return RES(6, hl);
        case 0xB7: return RES(6, a);
        case 0xB8: return RES(7, b);
        case 0xB9: return RES(7, c);
        case 0xBA: return RES(7, d);
        case 0xBB: return RES(7, e);
        case 0xBC: return RES(7, h);
        case 0xBD: return RES(7, l);
        case 0xBE: return RES(7, hl);
        case 0xBF: return RES(7, a);
        case 0xC0: return SET(0, b);
        case 0xC1: return SET(0, c);
        case 0xC2: return SET(0, d);
        case 0xC3: return SET(0, e);
        case 0xC4: return SET(0, h);
        case 0xC5: return SET(0, l);
        case 0xC6: return SET(0, hl);
        case 0xC7: return SET(0, a);
        case 0xC8: return SET(1, b);
        case 0xC9: return SET(1, c);
        case 0xCA: return SET(1, d);
        case 0xCB: return SET(1, e);
        case 0xCC: return SET(1, h);
        case 0xCD: return SET(1, l);
        case 0xCE: return SET(1, hl);
        case 0xCF: return SET(1, a);
        case 0xD0: return SET(2, b);
        case 0xD1: return SET(2, c);
        case 0xD2: return SET(2, d);
        case 0xD3: return SET(2, e);
        case 0xD4: return SET(2, h);
        case 0xD5: return SET(2, l);
        case 0xD6: return SET(2, hl);
        case 0xD7: return SET(2, a);
        case 0xD8: return SET(3, b);
        case 0xD9: return SET(3, c);
        case 0xDA: return SET(3, d);
        case 0xDB: return SET(3, e);
        case 0xDC: return SET(3, h);
        case 0xDD: return SET(3, l);
        case 0xDE: return SET(3, hl);
        case 0xDF: return SET(3, a);
        case 0xE0: return SET(4, b);
        case 0xE1: return SET(4, c);
        case 0xE2: return SET(4, d);
        case 0xE3: return SET(4, e);
        case 0xE4: return SET(4, h);
        case 0xE5: return SET(4, l);
        case 0xE6: return SET(4, hl);
        case 0xE7: return SET(4, a);
        case 0xE8: return SET(5, b);
        case 0xE9: return SET(5, c);
        case 0xEA: return SET(5, d);
        case 0xEB: return SET(5, e);
        case 0xEC: return SET(5, h);
        case 0xED: return SET(5, l);
        case 0xEE: return SET(5, hl);
        case 0xEF: return SET(5, a);
        case 0xF0: return SET(6, b);
        case 0xF1: return SET(6, c);
        case 0xF2: return SET(6, d);
        case 0xF3: return SET(6, e);
        case 0xF4: return SET(6, h);
        case 0xF5: return SET(6, l);
        case 0xF6: return SET(6, hl);
        case 0xF7: return SET(6, a);
        case 0xF8: return SET(7, b);
        case 0xF9: return SET(7, c);
        case 0xFA: return SET(7, d);
        case 0xFB: return SET(7, e);
        case 0xFC: return SET(7, h);
        case 0xFD: return SET(7, l);
        case 0xFE: return SET(7, hl);
        case 0xFF: return SET(7, a);
        default:
            throw std::runtime_error(
                std::format("Illegal CB opcode: 0xCB{:02X} at PC: 0x{:04X}", opcode, pc - 2)
            );
    }
}

// https://gbdev.io/pandocs/Interrupts.html#interrupt-handling
auto Cpu::handle_interrupt() -> u32 {
    const u8 pending_interrupt = get_pending_interrupt();

    // Interrupt priorities ---> https://gbdev.io/pandocs/Interrupts.html#interrupt-priorities
    // Bit 0 (VBlank) has the highest priority, and Bit 4 (Joypad) has the lowest priority.
    u8 bit = 0;
    for (int i = 0; i < 5; i++) {
        // Interrupt has been found, exit the loop now.
        if ((pending_interrupt >> i & 0b1) != 0) {
            bit = i;
            break;
        }
    }

    // The IF bit corresponding to this interrupt and the IME flag are reset by the CPU.
    // The former “acknowledges” the interrupt
    // and the latter prevents any further interrupts from being handled until the program re-enables them, typically by using the RETI instruction.
    interrupt_flag &= ~(1 << bit);
    ime = false;

    // The current value of the PC register is pushed onto the stack, consuming 2 more cycles.
    // BE MINDFUL THAT, GB IS LITTLE ENDIAN HERE
    // 0xABCD = [CD][AB]
    write_mmu(--sp, static_cast<u8>(pc >> 8));
    write_mmu(--sp, static_cast<u8>(pc & 0xFF));

    // Jump to interrupt handler ----> https://gbdev.io/pandocs/Interrupt_Sources.html
    switch (bit) {
        case 0: pc = 0x0040; break; //VBlank
        case 1: pc = 0x0048; break; //LCD Stat
        case 2: pc = 0x0050; break; //Timer
        case 3: pc = 0x0058; break; //Serial
        case 4: pc = 0x0060; break; //Joypad
        default: std::unreachable();
    }

    return 5;
}

auto Cpu::get_pending_interrupt() -> u8 {
    // if & ie & 0001 1111
    return interrupt_flag & interrupt_enable & INTERRUPT_MASK;
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
    // Execution of a STOP instruction stops both the system clock and oscillator circuit.

    // STOP mode is entered and the LCD controller also stops.
    state = CpuState::Stopped;

    // Resets timer's Divider register
    gb.timer.set_div(0);

    // Discard the second 0x00 byte here. because STOP opcode is 0x1000
    fetch_unsigned_8bit();
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
auto Cpu::JR_cc_s8(FlagCondition cc) -> u8 {
    if (cc.is_valid(f)) {
        return JR_s8();
    }

    fetch_unsigned_8bit();
    return 2;
}

auto Cpu::JP_n16() -> u8 {
    pc = fetch_unsigned_16bit();
    return 4;
}

auto Cpu::JP_cc_n16(FlagCondition cc) -> u8 {
    if (cc.is_valid(f)) {
        return JP_n16();
    }

    fetch_unsigned_16bit(); // Discard to advance PC
    return 3;
}

auto Cpu::JP_r16(RegisterPair reg_pair) -> u8 {
    pc = reg_pair.value();
    return 1;
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

auto Cpu::INC_m16(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto result = mem + 1;
    const auto flag_h = (mem & 0xF) == 0xF;
    write_mmu(reg_pair.value(), result);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    return 3;
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

auto Cpu::DEC_m16(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto result = mem - 1;
    write_mmu(reg_pair.value(), result);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, (mem & 0xF) == 0x0);
    return 3;
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

auto Cpu::LD_r8_m16(Register &reg_into, RegisterPair reg_pair_from) -> u8 {
    reg_into.value = read_mmu(reg_pair_from.value());
    return 2;
}

auto Cpu::LD_r8_a16(Register &reg) -> u8 {
    reg.value = read_mmu(fetch_unsigned_16bit());
    return 4;
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

auto Cpu::LD_a16_r8(Register reg) -> u8 {
    write_mmu(fetch_unsigned_16bit(), reg.value);
    return 4;
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

auto Cpu::LD_SP_r16(RegisterPair reg_pair) -> u8 {
    sp = reg_pair.value();
    return 2;
}

// Load to the HL register, 16-bit data calculated by adding the signed 8-bit operand e to the 16-bit value of the SP register.
auto Cpu::LD_HL_SP_s8() -> u8 {
    const auto s8 = fetch_signed_8bit();
    const auto flag_h = (sp & 0xF) + (s8 & 0xF) > 0xF;
    const auto flag_c = (sp & 0xFF) + (s8 & 0xFF) > 0xFF;
    hl.set(sp + s8);
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::LDH_a8_r8(Register reg) -> u8 {
    const auto byte = fetch_unsigned_8bit();
    write_mmu(0xFF00 | byte, reg.value);
    return 3;
}

auto Cpu::LDH_m8_r8(Register reg_into, Register reg_from) -> u8 {
    write_mmu(0xFF00 | reg_into.value, reg_from.value);
    return 2;
}

auto Cpu::LDH_r8_a8(Register &reg_into) -> u8 {
    reg_into.value = read_mmu(0xFF00 | fetch_unsigned_8bit());
    return 3;
}

auto Cpu::LDH_r8_m8(Register &reg_into, Register reg_from) -> u8 {
    reg_into.value = read_mmu(0xFF00 | reg_from.value);
    return 2;
}

auto Cpu::ADD_r8_r8(Register &reg_into, Register reg_from) -> u8 {
    const auto flag_h = (reg_into.value & 0xF) + (reg_from.value & 0xF) > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + reg_from.value > 0xFF;

    reg_into.value += reg_from.value;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 1;
}

auto Cpu::ADD_r8_m8(Register &reg_into) -> u8 {
    const auto mem = fetch_unsigned_8bit();
    const auto flag_h = (reg_into.value & 0xF) + (mem & 0xF) > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + mem > 0xFF;

    reg_into.value += mem;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::ADD_r8_m16(Register &reg_into, RegisterPair reg_pair_from) -> u8 {
    const auto mem = read_mmu(reg_pair_from.value());

    const auto flag_h = (reg_into.value & 0xF) + (mem & 0xF) > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + mem > 0xFF;

    reg_into.value += mem;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);

    return 2;
}

auto Cpu::ADC_r8_r8(Register &reg_into, Register reg_from) -> u8 {
    const auto flag_c_value = get_flag_value(Flag::C);
    const auto flag_h = (reg_into.value & 0xF) + (reg_from.value & 0xF) + flag_c_value > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + reg_from.value + flag_c_value > 0xFF;

    reg_into.value = reg_into.value + reg_from.value + flag_c_value;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);

    return 1;
}

auto Cpu::ADC_r8_n8(Register &reg_into) -> u8 {
    const auto n8 = fetch_unsigned_8bit();
    const auto flag_c_value = get_flag_value(Flag::C);
    const auto flag_h = (reg_into.value & 0xF) + (n8 & 0xF) + flag_c_value > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + n8 + flag_c_value > 0xFF;

    reg_into.value = reg_into.value + n8 + flag_c_value;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);

    return 2;
}

auto Cpu::ADC_r8_m16(Register &reg_into, RegisterPair reg_pair_from) -> u8 {
    const auto flag_c_value = get_flag_value(Flag::C);
    const auto mem = read_mmu(reg_pair_from.value());
    const auto flag_h = (reg_into.value & 0xF) + (mem & 0xF) + flag_c_value > 0xF;
    const auto flag_c = static_cast<u16>(reg_into.value) + mem + flag_c_value > 0xFF;

    reg_into.value = reg_into.value + mem + flag_c_value;

    set_flag_value(Flag::Z, reg_into.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);

    return 2;
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

auto Cpu::ADD_SP_s8() -> u8 {
    const auto s8 = fetch_signed_8bit();
    const bool flag_h = (sp & 0xF) + (s8 & 0xF) > 0xF;
    const bool flag_c = (sp & 0xFF) + (s8 & 0xFF) > 0xFF;
    sp += s8;
    set_flag_value(Flag::Z, false);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 4;
}

auto Cpu::SUB_r8(Register reg) -> u8 {
    const auto flag_h = (a.value & 0xF) < (reg.value & 0xF);
    const auto flag_c = a.value < reg.value;
    a.value -= reg.value;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 1;
}

auto Cpu::SUB_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto flag_h = (a.value & 0xF) < (mem & 0xF);
    const auto flag_c = a.value < mem;
    a.value -= mem;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::SUB_n8() -> u8 {
    const auto mem = fetch_unsigned_8bit();
    const auto flag_h = (a.value & 0xF) < (mem & 0xF);
    const auto flag_c = a.value < mem;
    a.value -= mem;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::SBC_r8(Register reg) -> u8 {
    const auto flag_c_value = get_flag_value(Flag::C);
    const auto flag_h = (a.value & 0xF) < ((reg.value & 0xF) + flag_c_value);
    const auto flag_c = a.value < (reg.value + flag_c_value);

    a.value = a.value - reg.value - flag_c_value;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 1;
}

auto Cpu::SBC_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto flag_c_value = get_flag_value(Flag::C);

    const auto flag_h = (a.value & 0xF) < ((mem & 0xF) + flag_c_value);
    const auto flag_c = a.value < (mem + flag_c_value);

    a.value = a.value - mem - flag_c_value;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::SBC_n8() -> u8 {
    const auto mem = fetch_unsigned_8bit();
    const auto flag_c_value = get_flag_value(Flag::C);

    const auto flag_h = (a.value & 0xF) < ((mem & 0xF) + flag_c_value);
    const auto flag_c = a.value < (mem + flag_c_value);

    a.value = a.value - mem - flag_c_value;

    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::AND_r8(Register reg) -> u8 {
    a.value &= reg.value;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, true);
    set_flag_value(Flag::C, false);
    return 1;
}

auto Cpu::AND_n8() -> u8 {
    a.value &= fetch_unsigned_8bit();
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, true);
    set_flag_value(Flag::C, false);
    return 2;
}

auto Cpu::AND_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    a.value &= mem;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, true);
    set_flag_value(Flag::C, false);
    return 2;
}

auto Cpu::XOR_r8(Register reg) -> u8 {
    a.value ^= reg.value;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 1;
}

auto Cpu::XOR_n8() -> u8 {
    a.value ^= fetch_unsigned_8bit();
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 2;
}

auto Cpu::XOR_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    a.value ^= mem;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 2;
}

auto Cpu::OR_r8(Register reg) -> u8 {
    a.value |= reg.value;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 1;
}

auto Cpu::OR_n8() -> u8 {
    a.value |= fetch_unsigned_8bit();
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 1;
}

auto Cpu::OR_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    a.value |= mem;
    set_flag_value(Flag::Z, a.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 2;
}

auto Cpu::CP_r8(Register reg) -> u8 {
    const auto flag_h = (a.value & 0xF) < (reg.value & 0xF);
    const auto flag_c = a.value < reg.value;
    const auto result = a.value - reg.value;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 1;
}

auto Cpu::CP_n8() -> u8 {
    const auto n8 = fetch_unsigned_8bit();
    const auto flag_h = (a.value & 0xF) < (n8 & 0xF);
    const auto flag_c = a.value < n8;
    const auto result = a.value - n8;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, true);
    set_flag_value(Flag::H, flag_h);
    set_flag_value(Flag::C, flag_c);
    return 2;
}

auto Cpu::CP_m16(RegisterPair reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto flag_h = (a.value & 0xF) < (mem & 0xF);
    const auto flag_c = a.value < mem;
    const auto result = a.value - mem;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, true);
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

// RL r: Rotate left (register)
auto Cpu::RL(Register &reg) -> u8 {
    const auto bit_7 = (reg.value >> 7) & 0b1;
    reg.value = reg.value << 1 | get_flag_value(Flag::C);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_7 != 0);
    return 2;
}

// RL (HL): Rotate left (indirect HL)
auto Cpu::RL(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_7 = (mem >> 7) & 0b1;
    const auto result = mem << 1 | get_flag_value(Flag::C);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_7 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// RLC r: Rotate left circular (register)
auto Cpu::RLC(Register &reg) -> u8 {
    const u8 bit7 = (reg.value >> 7) & 0b1;
    reg.value = reg.value << 1 | bit7;
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit7 != 0);
    return 2;
}

// RLC (HL): Rotate left circular (indirect HL)
auto Cpu::RLC(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const u8 bit_7 = (mem >> 7) & 0b1;
    const auto result = mem << 1 | bit_7;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_7 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// RR r: Rotate right (register)
auto Cpu::RR(Register &reg) -> u8 {
    const auto bit_0 = reg.value & 0b1;
    reg.value = reg.value >> 1 | (get_flag_value(Flag::C) << 7);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    return 2;
}

// RR (HL): Rotate right (indirect HL)
auto Cpu::RR(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_0 = mem & 0b1;
    const auto result = mem >> 1 | (get_flag_value(Flag::C) << 7);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// RRC r: Rotate right circular (register)
auto Cpu::RRC(Register &reg) -> u8 {
    const auto bit_0 = reg.value & 0b1;
    reg.value = reg.value >> 1 | (bit_0 << 7);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    return 2;
}

// RRC (HL): Rotate right circular (indirect HL)
auto Cpu::RRC(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_0 = mem & 0b1;
    const auto result = mem >> 1 | (bit_0 << 7);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// SLA r: Shift left arithmetic (register)
auto Cpu::SLA(Register &reg) -> u8 {
    const auto bit_7 = reg.value >> 7 & 0b1;
    reg.value = reg.value << 1 & (~0b1);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_7 != 0);
    return 2;
}

// SLA (HL): Shift left arithmetic (indirect HL)
auto Cpu::SLA(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_7 = mem >> 7 & 0b1;
    const auto result = mem << 1 & (~0b1);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_7 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// SRA r: Shift right arithmetic (register)
auto Cpu::SRA(Register &reg) -> u8 {
    const auto bit_7 = reg.value >> 7 & 0b1;
    const auto bit_0 = reg.value & 0b1;
    reg.value = reg.value >> 1 | (bit_7 << 7);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    return 2;
}

auto Cpu::SRA(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_7 = mem >> 7 & 0b1;
    const auto bit_0 = mem & 0b1;
    const auto result = mem >> 1 | (bit_7 << 7);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// Swaps the high and low 4-bit nibbles of the 8-bit register r.
auto Cpu::SWAP(Register &reg) -> u8 {
    const auto high = reg.value >> 4;
    const auto low = reg.value & 0xF;
    reg.value = low << 4 | high;
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    return 2;
}

// SWAP (HL): Swap nibbles (indirect HL)
auto Cpu::SWAP(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto high = mem >> 4;
    const auto low = mem & 0xF;
    const auto result = low << 4 | high;
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, false);
    write_mmu(reg_pair.value(), result);
    return 4;
}

auto Cpu::SRL(Register &reg) -> u8 {
    const auto bit_0 = reg.value & 0b1;
    reg.value = reg.value >> 1 & ~(1 << 7);
    set_flag_value(Flag::Z, reg.value == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    return 2;
}

auto Cpu::SRL(RegisterPair &reg_pair) -> u8 {
    const auto mem = read_mmu(reg_pair.value());
    const auto bit_0 = mem & 0b1;
    const auto result = mem >> 1 & ~(1 << 7);
    set_flag_value(Flag::Z, result == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, false);
    set_flag_value(Flag::C, bit_0 != 0);
    write_mmu(reg_pair.value(), result);
    return 2;
}

auto Cpu::BIT(u8 bit, Register &reg) -> u8 {
    assert(bit < 8);
    const auto chosen_bit = reg.value >> bit & 0b1;
    set_flag_value(Flag::Z, chosen_bit == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, true);
    return 2;
}

auto Cpu::BIT(u8 bit, RegisterPair reg_pair) -> u8 {
    assert(bit < 8);
    const auto chosen_bit = read_mmu(reg_pair.value()) >> bit & 0b1;
    set_flag_value(Flag::Z, chosen_bit == 0);
    set_flag_value(Flag::N, false);
    set_flag_value(Flag::H, true);
    return 3;
}

// Resets the bit b of the 8-bit register r to 0.
auto Cpu::RES(u8 bit, Register &reg) -> u8 {
    assert(bit < 8);
    reg.value &= ~(1 << bit);
    return 2;
}

auto Cpu::RES(u8 bit, RegisterPair &reg_pair) -> u8 {
    assert(bit < 8);
    const auto mem = read_mmu(reg_pair.value());
    const auto result = mem & ~(1 << bit);
    write_mmu(reg_pair.value(), result);
    return 4;
}

// Sets the bit b of the 8-bit register r to 1.
auto Cpu::SET(u8 bit, Register &reg) -> u8 {
    assert(bit < 8);
    reg.value |= (1 << bit);
    return 2;
}

auto Cpu::SET(u8 bit, RegisterPair reg_pair) -> u8 {
    assert(bit < 8);
    const auto mem = read_mmu(reg_pair.value());
    const auto result = mem | (1 << bit);
    write_mmu(reg_pair.value(), result);
    return 4;
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

auto Cpu::RET() -> u8 {
    const auto lsb = read_mmu(sp++);
    const auto msb = read_mmu(sp++);
    pc = (static_cast<u16>(msb) << 8) | lsb;
    return 4;
}

auto Cpu::RET(FlagCondition cc) -> u8 {
    if (cc.is_valid(f)) {
        RET();
        return 5;
    }
    return 2;
}

auto Cpu::RETI() -> u8 {
    ime = true;
    return RET();
}

auto Cpu::POP(RegisterPair reg_pair) -> u8 {
    const auto lsb = read_mmu(sp++);
    const auto msb = read_mmu(sp++);
    reg_pair.set((static_cast<u16>(msb) << 8) | lsb);
    return 3;
}

auto Cpu::PUSH(RegisterPair reg_pair) -> u8 {
    const auto msb = static_cast<u8>(reg_pair.value() >> 8);
    const auto lsb = static_cast<u8>(reg_pair.value() & 0xFF);
    write_mmu(--sp, msb);
    write_mmu(--sp, lsb);
    return 4;
}


auto Cpu::CALL() -> u8 {
    const auto address = fetch_unsigned_16bit();
    write_mmu(--sp, static_cast<u8>(pc >> 8));
    write_mmu(--sp, static_cast<u8>(pc));
    pc = address;
    return 6;
}

auto Cpu::CALL_cc(FlagCondition cc) -> u8 {
    if (cc.is_valid(f)) {
        return CALL();
    } else {
        fetch_unsigned_16bit();
        return 3;
    }
}

auto Cpu::RST(u8 rst_number) -> u8 {
    assert(rst_number <= 7);

    write_mmu(--sp, static_cast<u8>(pc >> 8));
    write_mmu(--sp, static_cast<u8>(pc & 0xFF));
    pc = rst_number * 0x08; // PC = unsigned_16(lsb=n, msb=0x00)
    return 4;
}

auto Cpu::HALT() -> u8 {
    state = CpuState::Halted;
    return 1;
}

// Disables interrupt handling by setting IME=0 and cancelling any scheduled effects of the EI instruction if any
auto Cpu::DI() -> u8 {
    ime = false;
    ime_next = false;
    return 1;
}

// Schedules interrupt handling to be enabled after the next machine cycle.
auto Cpu::EI() -> u8 {
    ime_next = true;
    return 1;
}

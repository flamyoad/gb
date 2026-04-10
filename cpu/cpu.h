//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "register.h"
#include "../common_types.h"

class Gameboy;

class Cpu {
public:
        explicit Cpu(Gameboy &gb);
        auto tick() -> u32;

private:
        Gameboy& gb;

        /*
        https://meganesu.github.io/generate-gb-opcodes/
        */
        // Registers
        Register a, b, c, d, e, h, l;
        FlagRegister f; // [7]=Z [6]=N [5]=H [4]=C  [3:0]= Hardware enforced always 0
        RegisterPair af, bc, de, hl;

        u16 pc;
        u16 sp;

        auto set_flag(u8 mask, bool flag_value) -> void;

        // CPU fetch decode operations
        auto execute(u8 opcode) -> u32;
        auto execute_cb_opcode(u8 opcode) -> u32;

        auto fetch_8bit() -> u8;
        auto fetch_16bit() -> u16;

        auto read_mmu(u16 address) -> u8;
        auto write_mmu(u16 address, u8 value) -> void;

        // r8 = register,
        // r16 = register pair
        // n8 = 1-byte immediate data,
        // n16 = 2-byte immediate data
        // m16 = memory location specified by register pair
        auto NOP() -> u8;
        auto INC_r8(Register &reg) -> u8;
        auto INC_r16(RegisterPair &reg_pair) -> u8;
        auto DEC_r8(Register &reg) -> u8;
        auto DEC_r16(RegisterPair &reg_pair) -> u8;
        auto LD_r8_n8(Register &reg) -> u8;
        auto LD_r8_r8(Register &reg_into, Register reg_from) -> u8;
        auto LD_r16_n8(Register &reg, u8 value) -> u8;
        auto LD_r16_n16(RegisterPair &reg_pair, u16 value) -> u8;
        auto LD_m16_r8(RegisterPair &reg_pair, Register reg) -> u8;
        auto LD_r8_r16(Register &reg, RegisterPair reg_pair) -> u8;
        auto LD_n16_SP() -> u8;

        auto ADD_HL_r16(RegisterPair reg_pair) -> u8;

        auto RLCA() -> u8;
        // auto alu_add
};

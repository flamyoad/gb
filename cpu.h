//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "common_types.h"

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
        u8 a, b, c, d, e, h, l;
        u8 f; // [7]=Z [6]=N [5]=H [4]=C  [3:0]= Hardware enforced always 0
        u16 pc;
        u16 sp;

        // Register pairs
        u16 af() const;
        u16 bc() const;
        u16 de() const;
        u16 hl() const;

        void set_af(u16 value);
        void set_bc(u16 value);
        void set_de(u16 value);
        void set_hl(u16 value);

        // Flags
        static constexpr u8 FLAG_Z = 1 << 7; // Zero flag
        static constexpr u8 FLAG_N = 1 << 6; // INC/DEC flag (BCD)
        static constexpr u8 FLAG_H = 1 << 5; // Half-carry flag (BCD)
        static constexpr u8 FLAG_C = 1 << 4; // Carry flag;

        void set_flag(u8 mask, bool value);

        // CPU fetch decode operations
        auto execute(u8 opcode) -> u32;
        auto execute_cb_opcode(u8 opcode) -> u32;

        auto fetch_8bit() -> u8;
        auto fetch_16bit() -> u16;

        auto read_mmu(u16 address) -> u8;
        auto write_mmu(u16 address, u8 value) -> void;

        // todo: move those to new class once bigger
        auto INC_r8(u8 &reg) -> u8;
        auto DEC_r8(u8 &reg) -> u8;
        auto LD_r8_n8(u8 &reg) -> u8;
        auto LD_r8_r8(u8 &reg_into, u8 reg_from) -> u8;
        auto LD_n16_SP() -> u8;

        auto ADD_HL_rr(u16 rr) -> u8;

        auto RLCA() -> u8;
        // auto alu_add
};

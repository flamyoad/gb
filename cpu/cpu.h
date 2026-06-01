//
// Created by Ng Zhen Hao on 21/03/2026.
//
#pragma once

#include "flag_condition.h"
#include "interrupt.h"
#include "register.h"
#include "../common_types.h"

class Gameboy;

class Cpu {
public:
        explicit Cpu(Gameboy &gb);
        u8 interrupt_flag;
        u8 interrupt_enable;
        auto tick() -> u32;
        auto request_interrupt(Interrupt interrupt) -> void;

private:
        Gameboy& gb;

        /*
        https://meganesu.github.io/generate-gb-opcodes/
        https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595
        */
        // Registers
        Register a, b, c, d, e, h, l;
        FlagRegister f; // [7]=Z [6]=N [5]=H [4]=C  [3:0]= Hardware enforced always 0
        RegisterPair af, bc, de, hl;

        u16 pc;
        u16 sp;
        bool ime; // Interrupt Master Enable flag
        bool ime_next;
        bool halted;

        auto get_flag_value(Flag flag) -> u8;
        auto set_flag_value(Flag flag, bool flag_value) -> void;

        // CPU fetch decode operations
        auto execute(u8 opcode) -> u32;
        auto execute_cb_opcode(u8 opcode) -> u32;

        // Interrupt handling
        auto handle_interrupt() -> u32;
        auto get_pending_interrupt() -> u8;

        auto fetch_unsigned_8bit() -> u8;
        auto fetch_unsigned_16bit() -> u16;
        auto fetch_signed_8bit() -> i8;

        auto read_mmu(u16 address) -> u8;
        auto write_mmu(u16 address, u8 value) -> void;

        // r8 = register,
        // r16 = register pair
        // n8 = Unsigned 1-byte immediate data obtained from PC
        // n16 = Unsigned 2-byte immediate data obtained from PC
        // s8 = Signed 1-byte  immediate data obtained from PC
        // m8 = content of main memory by reading address specified by register
        // m16 = content of main memory by reading address specified by register pair
        // a8 = content of main memory by reading address specified by the 1-byte immediate data
        // a16 = content of main memory by reading address specified by the 2-byte immediate data
        auto NOP() -> u8;
        auto STOP() -> u8;
        auto JR_s8() -> u8;
        auto JR_cc_s8(FlagCondition cc) -> u8;
        auto JP_n16() -> u8;
        auto JP_cc_n16(FlagCondition cc) -> u8;
        auto JP_r16(RegisterPair reg_pair) -> u8;
        auto INC_r8(Register &reg) -> u8;
        auto INC_r16(RegisterPair &reg_pair) -> u8;
        auto INC_m16(RegisterPair &reg_pair) -> u8;
        auto INC_SP() -> u8;
        auto DEC_r8(Register &reg) -> u8;
        auto DEC_r16(RegisterPair &reg_pair) -> u8;
        auto DEC_m16(RegisterPair &reg_pair) -> u8;
        auto DEC_SP() -> u8;
        auto LD_r8_n8(Register &reg) -> u8;
        auto LD_r8_r8(Register &reg_into, Register reg_from) -> u8;
        auto LD_r8_m16(Register &reg_into, RegisterPair reg_pair) -> u8;
        auto LD_r8_a16(Register &reg) -> u8;
        auto LD_m16_n8(RegisterPair &reg_pair) -> u8;
        auto LD_r16_n16(RegisterPair &reg_pair) -> u8;
        auto LD_m16_r8(RegisterPair &reg_pair, Register reg) -> u8;
        auto LD_r8_r16(Register &reg, RegisterPair reg_pair) -> u8;
        auto LD_n16_SP() -> u8;
        auto LD_SP_r16(RegisterPair reg_pair) -> u8;
        auto LD_SP_n16() -> u8;
        auto LD_HL_SP_s8() -> u8;
        auto LD_A_HLdec() -> u8;
        auto LD_HLdec_A() -> u8;
        auto LD_A_HLinc() -> u8;
        auto LD_HLinc_A() -> u8;
        auto LDH_a8_r8(Register reg) -> u8;
        auto LDH_m8_r8(Register reg_into, Register reg_from) -> u8;
        auto LDH_r8_a8(Register &reg) -> u8;
        auto LDH_r8_m8(Register &reg_into, Register reg_from) -> u8;
        auto LD_a16_r8(Register reg) -> u8;

        auto ADD_r8_r8(Register &reg_into, Register reg_from) -> u8;
        auto ADD_r8_m8(Register &reg_into) -> u8;
        auto ADD_r8_m16(Register &reg_into, RegisterPair reg_pair_from) -> u8;
        auto ADC_r8_r8(Register &reg_into, Register reg_from) -> u8;
        auto ADC_r8_n8(Register &reg_into) -> u8;
        auto ADC_r8_m16(Register &reg_into, RegisterPair reg_pair_from) -> u8;
        auto ADD_HL_r16(RegisterPair reg_pair) -> u8;
        auto ADD_HL_SP() -> u8;
        auto ADD_SP_s8() -> u8;

        auto SUB_r8(Register reg) -> u8;
        auto SUB_m16(RegisterPair reg_pair) -> u8;
        auto SUB_n8() -> u8;
        auto SBC_r8(Register reg) -> u8;
        auto SBC_m16(RegisterPair reg_pair) -> u8;
        auto SBC_n8() -> u8;

        auto AND_r8(Register reg) -> u8;
        auto AND_n8() -> u8;
        auto AND_m16(RegisterPair reg_pair) -> u8;
        auto XOR_r8(Register reg) -> u8;
        auto XOR_n8() -> u8;
        auto XOR_m16(RegisterPair reg_pair) -> u8;
        auto OR_r8(Register reg) -> u8;
        auto OR_n8() -> u8;
        auto OR_m16(RegisterPair reg_pair) -> u8;

        auto CP_r8(Register reg) -> u8;
        auto CP_n8() -> u8;
        auto CP_m16(RegisterPair reg_pair) -> u8;

        auto RLCA() -> u8;
        auto RRCA() -> u8;
        auto RLA() -> u8;
        auto RRA() -> u8;
        auto RL(Register &reg) -> u8;
        auto RL(RegisterPair &reg_pair) -> u8;
        auto RLC(Register &reg) -> u8;
        auto RLC(RegisterPair &reg_pair) -> u8;
        auto RR(Register &reg) -> u8;
        auto RR(RegisterPair &reg_pair) -> u8;
        auto RRC(Register &reg) -> u8;
        auto RRC(RegisterPair &reg_pair) -> u8;

        auto SLA(Register &reg) -> u8;
        auto SLA(RegisterPair &reg_pair) -> u8;
        auto SRA(Register &reg) -> u8;
        auto SRA(RegisterPair &reg_pair) -> u8;
        auto SWAP(Register &reg) -> u8;
        auto SWAP(RegisterPair &reg_pair) -> u8;
        auto SRL(Register &reg) -> u8;
        auto SRL(RegisterPair &reg_pair) -> u8;
        auto BIT(u8 bit, Register &reg) -> u8;
        auto BIT(u8 bit, RegisterPair reg_pair) -> u8;
        auto RES(u8 bit, Register &reg) -> u8;
        auto RES(u8 bit, RegisterPair &reg_pair) -> u8;
        auto SET(u8 bit, Register &reg) -> u8;
        auto SET(u8 bit, RegisterPair reg_pair) -> u8;

        auto DAA() -> u8;
        auto CPL() -> u8;
        auto CCF() -> u8;
        auto SCF() -> u8;

        auto RET() -> u8;
        auto RET(FlagCondition cc) -> u8;
        auto RETI() -> u8;
        auto POP(RegisterPair reg_pair) -> u8;
        auto PUSH(RegisterPair reg_pair) -> u8;
        auto CALL() -> u8;
        auto CALL_cc(FlagCondition cc) -> u8;
        auto RST(u8 rst_number) -> u8;
        auto HALT() -> u8;
        auto DI() -> u8;
        auto EI() -> u8;
};

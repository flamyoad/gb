//
// Created by Ng Zhen Hao on 10/04/2026.
//

#pragma once

#include "../common_types.h"

// Register
class Register {
public:
    Register();
    virtual ~Register() = default;
    u8 value;
    auto operator ==(u8 other) const -> bool;
};

// FlagRegister
class FlagRegister: public Register {
public:
    auto set(u8 mask, bool flag_value) -> void;
};

enum class Flag : u8 {
    Z = 1 << 7, // Zero flag
    N = 1 << 6, // INC/DEC flag (BCD)
    H = 1 << 5, // Half-carry flag (BCD)
    C = 1 << 4, // Carry flag;
};


// RegisterPair
class RegisterPair {
public:
    RegisterPair(Register& hi, Register& lo);
    auto set(u16 value) const -> void;
    auto value() const -> u16;
    auto operator ==(u16 other) const -> bool;

private:
    Register& hi;
    Register& lo;
};

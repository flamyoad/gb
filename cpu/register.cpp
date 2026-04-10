//
// Created by Ng Zhen Hao on 10/04/2026.
//

#include "register.h"
#include <typeinfo>

// Register
Register::Register(): value(0) {}

auto Register::operator==(const u8 other) const -> bool {
    return this->value == other;
}

// Flag Register
auto FlagRegister::set(const u8 mask, bool flag_value) -> void {
    if (flag_value) {
        value |= mask;
    } else {
        value &= ~mask;
    }
}

// RegisterPair
RegisterPair::RegisterPair(Register &hi, Register &lo)
    : hi(hi), lo(lo) {
}

auto RegisterPair::set(const u16 value) const -> void {
    hi.value = value >> 8;

    if (typeid(lo) == typeid(FlagRegister)) {
        // Bit 0 to 3 must always be zero for flag registers
        lo.value = value & 0xF0;
    } else {
        lo.value = value & 0xFF;
    }
}

auto RegisterPair::value() const -> u16 {
    return static_cast<u16>(hi.value) << 8 | lo.value;
}

auto RegisterPair::operator ==(u16 other) const -> bool {
    const u16 register_value = static_cast<u16>(hi.value) << 8 | lo.value;
    return register_value == other;
}

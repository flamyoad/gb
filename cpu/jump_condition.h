//
// Created by Ng Zhen Hao on 12/04/2026.
//

#pragma once

#include "register.h"

enum class Condition {
    isOne, // flag is 1
    isZero, // flag is 0
};

class JumpCondition {
public:
    JumpCondition(const Flag flag, const Condition condition): flag(flag), condition(condition) {}

    auto is_valid(FlagRegister f) -> bool {
        auto flag_value = f.value & static_cast<u8>(flag) ? 1 : 0;
        if (condition == Condition::isZero && flag_value == 0) {
            return true;
        }
        if (condition == Condition::isOne && flag_value == 1) {
            return true;
        }
        return false;
    }

private:
    Flag flag;
    Condition condition;
};

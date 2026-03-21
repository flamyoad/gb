//
// Created by Ng Zhen Hao on 21/03/2026.
//

#include "mmu.h"
#include "bios.h"

Mmu::Mmu(Gameboy &gameboy) {
}

/**
0000-3FFF 16KB ROM Bank 00 (in cartridge, fixed at bank 00)
4000-7FFF 16KB ROM Bank 01..NN (in cartridge, switchable bank number)
8000-9FFF 8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
A000-BFFF 8KB External RAM (in cartridge, switchable bank, if any)
C000-CFFF 4KB Work RAM Bank 0 (WRAM)
D000-DFFF 4KB Work RAM Bank 1 (WRAM) (switchable bank 1-7 in CGB Mode)
E000-FDFF Same as C000-DDFF (ECHO) (typically not used)
FE00-FE9F Sprite Attribute Table (OAM)
FEA0-FEFF Not Usable
FF00-FF7F I/O Ports
FF80-FFFE High RAM (HRAM)
FFFF Interrupt Enable Register
 */
auto Mmu::read(u16 address) -> u8 {
    if (address <= 0x3FFF) {
        // please note bios is only 256
        if (address <= 0x00FF && !bios_check_successful()) {
            return BIOS[address];
        }

        // todo: BIOS check successful, should read from cartridge here and return values
    }

    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return 0xFF;
    }

    return mem[address];
}

void Mmu::write(u16 address, u8 value) {
    if (address >= 0xFF00 && address <= 0xFF7F) {
        mem[address] = value;
    }
}

auto Mmu::bios_check_successful() -> bool {
    return mem[0xFF50] != 0;
}

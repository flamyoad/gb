#include <iostream>

#include "gameboy.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom.gb>" << std::endl;
        return 1;
    }
    auto gameBoy = Gameboy();
    gameBoy.load_rom(std::string(argv[1]));

    while (true) {
        gameBoy.tick();
    }
}

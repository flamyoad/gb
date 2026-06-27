#include <iostream>
#include <string>

#include "gameboy.h"
#include <CLI/CLI.hpp>

int main(int argc, char* argv[]) {
    auto app = CLI::App{"Gb"};
    std::string rom_path;
    bool print_logs = false;

    app.add_option("rom", rom_path, "Path to the ROM");
    app.add_flag("--printLogs", print_logs, "Print debug logs of CPU");

    CLI11_PARSE(app, argc, argv);

    if (rom_path.empty()) {
        std::cerr << "Error: No ROM specified." << std::endl;
    }

    auto gameBoy = Gameboy();
    gameBoy.load_rom(rom_path);

    while (true) {
        gameBoy.tick();
    }
}

#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include "console_ui.hpp"
#include "game.hpp"

using namespace citadels;

namespace {
    // Reads one non-empty line; throws at end of input.
    std::string askName(int seat) {
        while (true) {
            std::cout << "Name of player " << (seat + 1) << ": " << std::flush;
            std::string name;
            if (!std::getline(std::cin, name)) {
                throw std::runtime_error("end of input while reading player names");
            }
            if (!name.empty() && name.back() == '\r') name.pop_back();
            if (!name.empty()) return name;
            std::cout << "The name cannot be empty.\n";
        }
    }
}

// Usage: citadels [path/to/districts.csv]
int main(int argc, char* argv[]) {
    try {
        ConsoleUI ui(std::cin, std::cout);

        const int players = 2 + ui.choose("How many players?", {"2 players", "3 players", "4 players", "5 players", "6 players", "7 players"});

        GameConfig config;
        for (int seat = 0; seat < players; ++seat) {
            config.playerNames.push_back(askName(seat));
        }
        config.districtsFile = argc > 1 ? argv[1] : "data/districts.csv";
        config.seed = std::random_device{}();

        Game(config, ui).play();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

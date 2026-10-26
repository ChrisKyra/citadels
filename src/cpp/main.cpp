#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include "console_ui.hpp"
#include "game.hpp"
#include "protocol_ui.hpp"

using namespace citadels;

namespace {
    // Bad command line or bad input from the program that starts us; exit code 2.
    class UsageError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    struct Options {
        bool protocol = false;                 // talk to the graphical program instead of a human
        int players = 0;                       // protocol mode only: names follow on stdin
        std::string data = "data/districts.csv";
        std::optional<std::uint32_t> seed;     // none = a different shuffle every time
    };

    bool startsWith(const std::string& text, const std::string& start) {
        return text.compare(0, start.size(), start) == 0;
    }

    // Whole-text unsigned integer up to `maximum`: rejects "", "-1", "12x", " 5".
    unsigned long parseNumber(const std::string& flag, const std::string& text, unsigned long maximum) {
        if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
            throw UsageError(flag + " needs a whole number, got \"" + text + "\"");
        }
        unsigned long value = 0;
        try {
            value = std::stoul(text);
        } catch (const std::out_of_range&) {
            throw UsageError(flag + " is too big: " + text);
        }
        if (value > maximum) {
            throw UsageError(flag + " is too big: " + text);
        }
        return value;
    }

    // Usage: citadels [path/to/districts.csv]
    //        citadels [--protocol --players N] [--seed S] [--data path/to/districts.csv]
    // The first form is the old one and is kept. In protocol mode the N player names follow on
    // stdin, one per line, and nothing but protocol lines goes to stdout.
    Options parseArguments(int argc, char* argv[]) {
        Options options;
        if (argc > 1 && !startsWith(argv[1], "--")) {
            options.data = argv[1];
            if (argc > 2) {
                throw UsageError("too many arguments");
            }
            return options;
        }

        for (int i = 1; i < argc; ++i) {
            const std::string flag = argv[i];
            if (flag == "--protocol") {
                options.protocol = true;
                continue;
            }
            if (flag != "--players" && flag != "--seed" && flag != "--data") {
                throw UsageError("unknown argument " + flag);
            }
            if (i + 1 >= argc) {
                throw UsageError(flag + " needs a value");
            }
            const std::string value = argv[++i];
            if (flag == "--players") {
                options.players = static_cast<int>(parseNumber(flag, value, 1000));
            } else if (flag == "--seed") {
                options.seed = static_cast<std::uint32_t>(parseNumber(flag, value, 4294967295UL));
            } else {
                options.data = value;
            }
        }

        if (options.protocol && (options.players < 2 || options.players > 7)) {
            throw UsageError("--protocol needs --players with a number from 2 to 7");
        }
        if (!options.protocol && options.players != 0) {
            throw UsageError("--players only works together with --protocol");
        }
        return options;
    }

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

    // Protocol mode: the name arrives on stdin without a question (stdout belongs to the protocol).
    std::string readName(int seat) {
        std::string name;
        if (!std::getline(std::cin, name)) {
            throw UsageError("end of input while reading the name of player " + std::to_string(seat + 1));
        }
        if (!name.empty() && name.back() == '\r') name.pop_back();
        if (name.empty()) {
            throw UsageError("the name of player " + std::to_string(seat + 1) + " is empty");
        }
        return name;
    }

    void playInTerminal(const Options& options) {
        ConsoleUI ui(std::cin, std::cout);

        const int players = 2 + ui.choose("How many players?", {"2 players", "3 players", "4 players", "5 players", "6 players", "7 players"});

        GameConfig config;
        for (int seat = 0; seat < players; ++seat) {
            config.playerNames.push_back(askName(seat));
        }
        config.districtsFile = options.data;
        config.seed = options.seed.value_or(std::random_device{}());

        Game(config, ui).play();
    }

    void playForGui(const Options& options) {
        GameConfig config;
        for (int seat = 0; seat < options.players; ++seat) {
            config.playerNames.push_back(readName(seat));
        }
        config.districtsFile = options.data;
        config.seed = options.seed.value_or(std::random_device{}());

        ProtocolUI ui(std::cin, std::cout);
        Game(config, ui).play();
    }
}

int main(int argc, char* argv[]) {
    try {
        const Options options = parseArguments(argc, argv);
        if (options.protocol) {
            playForGui(options);
        } else {
            playInTerminal(options);
        }
    } catch (const UsageError& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

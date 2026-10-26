#include "console_ui.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include "game_state.hpp"

namespace citadels {

    namespace {
        // Returns false at end of input; strips a trailing '\r' from CRLF input.
        bool readLine(std::istream& in, std::string& line) {
            if (!std::getline(in, line)) {
                return false;
            }
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return true;
        }

        std::string describe(const Card& card) {
            return card.name + " (" + std::to_string(card.cost) + ")";
        }

        // Whole-line integer only: rejects "abc", "2x", "1.5" and values that overflow int.
        bool parseInt(const std::string& text, int& value) {
            try {
                std::size_t pos = 0;
                value = std::stoi(text, &pos);
                return pos == text.size();
            } catch (const std::invalid_argument&) {
                return false;
            } catch (const std::out_of_range&) {
                return false;
            }
        }
    }

    ConsoleUI::ConsoleUI(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

    void ConsoleUI::announce(const std::string& msg) {
        out_ << msg << '\n';
    }

    void ConsoleUI::showTable(const GameState& state) {
        out_ << "Round " << state.roundNumber << '\n';
        for (const Player& player : state.players) {
            out_ << player.name;
            if (player.seat == state.crownHolder) {
                out_ << " [Crown]";
            }
            out_ << ": " << player.gold << " gold, " << player.hand.size() << " cards\n";
            out_ << "  City:";
            if (player.city.size() == 0) {
                out_ << " (empty)";
            }
            for (int i = 0; i < player.city.size(); ++i) {
                out_ << ' ' << describe(player.city.at(i));
                if (i + 1 < player.city.size()) out_ << ',';
            }
            out_ << '\n';
        }
    }

    void ConsoleUI::showPrivate(const Player& player) {
        out_ << player.name << ": " << player.gold << " gold\n";
        out_ << "Hand:";
        if (player.hand.size() == 0) {
            out_ << " (empty)";
        }
        for (int i = 0; i < player.hand.size(); ++i) {
            out_ << ' ' << describe(player.hand.at(i));
            if (i + 1 < player.hand.size()) out_ << ',';
        }
        out_ << '\n';
    }

    void ConsoleUI::handOff(const Player& player) {
        out_ << "\x1b[2J\x1b[H";
        out_ << "Pass the device to " << player.name << ". Press Enter when ready.\n";
        std::string line;
        if (!readLine(in_, line)) {
            throw std::runtime_error("ConsoleUI: end of input during hand-off");
        }
    }

    int ConsoleUI::choose(const std::string& prompt, const std::vector<std::string>& options) {
        if (options.empty()) {
            throw std::invalid_argument("ConsoleUI::choose: no options");
        }
        const int count = static_cast<int>(options.size());
        while (true) {
            out_ << prompt << '\n';
            for (int i = 0; i < count; ++i) {
                out_ << (i + 1) << ") " << options[static_cast<std::size_t>(i)] << '\n';
            }
            std::string line;
            if (!readLine(in_, line)) {
                throw std::runtime_error("ConsoleUI::choose: end of input");
            }
            int n = 0;
            if (parseInt(line, n) && n >= 1 && n <= count) {
                return n - 1;
            }
            out_ << "Please enter a number from 1 to " << count << ".\n";
        }
    }

    bool ConsoleUI::confirm(const std::string& prompt) {
        while (true) {
            out_ << prompt << " (y/n)\n";
            std::string line;
            if (!readLine(in_, line)) {
                throw std::runtime_error("ConsoleUI::confirm: end of input");
            }
            std::transform(line.begin(), line.end(), line.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (line == "y" || line == "yes") return true;
            if (line == "n" || line == "no") return false;
            out_ << "Please answer y or n.\n";
        }
    }
}

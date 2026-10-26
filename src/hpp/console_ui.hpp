#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include "user_interface.hpp"

namespace citadels {

    // Text interface over any pair of streams, so tests can drive it with stringstreams.
    class ConsoleUI final : public IUserInterface {
    public:
        ConsoleUI(std::istream& in, std::ostream& out);

        void announce(const std::string& msg) override;
        void handOff(const Player& player) override;
        int choose(const std::string& prompt, const std::vector<std::string>& options) override;
        bool confirm(const std::string& prompt) override;
        void showTable(const GameState& state) override;
        void showPrivate(const Player& player) override;

    private:
        std::istream& in_;
        std::ostream& out_;
    };
}

#pragma once

#include <vector>
#include <string>
#include "player.hpp"

namespace citadels{

    class GameState;

    class IUserInterface{
    public:
        virtual ~IUserInterface() = default;

        virtual void announce(const std::string& msg) = 0;
        virtual void handOff(const Player& player) = 0;
        virtual int choose(const std::string& prompt, const std::vector<std::string>& options) = 0;
        virtual bool confirm(const std::string& prompt) = 0;

        // Public information about the whole table; reads no input.
        virtual void showTable(const GameState& state) = 0;
        // What one player may see (their gold and hand); reads no input.
        virtual void showPrivate(const Player& player) = 0;

    };
}
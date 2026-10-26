#pragma once

#include "game_state.hpp"
#include "user_interface.hpp"
#include "player.hpp"
#include "characters.hpp"
#include "turn_context.hpp"
#include <vector>

namespace citadels {

    // One player's turn: take an action, then build / collect income until they end it.
    // Never calls handOff; passing the device is the Round's job.
    class Turn {
    public:
        Turn(GameState& state, RoundState& round, IUserInterface& ui, Player& player, const Character& character);

        void play();

    private:
        void onReveal();
        void takeAction();
        void mainPhase();
        void endOfTurn();
        void explainDistricts();
        // Thieves' Den: asks how many cards to pay with and which ones; returns hand indexes.
        std::vector<int> askPayment(int handIndex);

        TurnContext ctx_;
    };
}

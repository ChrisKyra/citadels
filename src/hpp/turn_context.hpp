#pragma once

#include "game_state.hpp"
#include "round_state.hpp"
#include "user_interface.hpp"
#include "player.hpp"
#include "characters.hpp"
#include <string>
#include <vector>

namespace citadels {

    // What one player's turn needs: the shared game, this round's events, the UI, who is playing and as whom,
    // plus what has already happened this turn.
    struct TurnContext {
        TurnContext(GameState& state, RoundState& round, IUserInterface& ui, Player& player, const Character& character)
            : state(state), round(round), ui(ui), player(player), character(character),
              buildsLeft(character.buildLimit()) {}

        GameState& state;
        RoundState& round;
        IUserInterface& ui;
        Player& player;
        const Character& character;

        bool actionTaken = false;
        bool powerUsed = false;
        bool incomeTaken = false;
        int buildsLeft;
        // Ids of the once-per-turn district abilities already used this turn.
        std::vector<std::string> abilitiesUsed;
    };
}

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "game_state.hpp"
#include "random.hpp"
#include "round.hpp"
#include "score_calculator.hpp"
#include "user_interface.hpp"

namespace citadels {

    struct GameConfig {
        std::vector<std::string> playerNames;   // seat i = playerNames[i]
        std::string districtsFile;              // path of districts.csv
        std::uint32_t seed = 1;                 // seeds the one Random of the game
        int districtsToFinish = 8;
    };

    // A whole game: setup, rounds until a city is complete, then scoring.
    // Not copyable or movable: it keeps a Random and a reference to the UI.
    class Game {
    public:
        // Loads the districts, shuffles, deals 4 cards and 2 gold to each player, crown on seat 0.
        // Throws std::invalid_argument for an unplayable number of players and
        // std::runtime_error if the districts file cannot be read.
        Game(const GameConfig& config, IUserInterface& ui);

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        // playRound() until the end is triggered, then finish().
        std::vector<ScoreBreakdown> play();
        // One round. Runs Round's steps itself so the ranks can be saved before endRound().
        void playRound();
        // Announces the scoreboard (the winner last) and returns the ranking, best first.
        std::vector<ScoreBreakdown> finish();

        GameState& state() { return state_; }
        const GameState& state() const { return state_; }
        // Indexed by seat: the highest character rank held in the last round (0 = none).
        const std::vector<int>& lastRoundRanks() const { return lastRoundRanks_; }

    private:
        IUserInterface& ui_;
        Random rng_;
        GameState state_;
        Round round_;
        std::vector<int> lastRoundRanks_;
    };
}

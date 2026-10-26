#pragma once

#include "game_state.hpp"
#include "user_interface.hpp"
#include "random.hpp"
#include "round_state.hpp"
#include "character_draft.hpp"

namespace citadels {

    // One round: set characters aside, draft, call each character in rank order, then clean up.
    class Round {
    public:
        // Throws std::invalid_argument for an unsupported player count.
        Round(GameState& state, IUserInterface& ui, Random& rng);

        void play();

        void removeCharacters();
        void chooseCharacters();
        void callCharacters();
        void endRound();

        RoundState& roundState() { return rs_; }
        const RoundState& roundState() const { return rs_; }

    private:
        GameState& state_;
        IUserInterface& ui_;
        Random& rng_;
        RoundState rs_;
        DraftPtr draft_;
    };
}

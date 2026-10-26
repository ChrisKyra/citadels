#pragma once

#include <string>
#include <vector>
#include "game_state.hpp"
#include "player.hpp"

namespace citadels {

    struct ScoreBreakdown {
        int seat = 0;
        std::string name;
        int districtPoints = 0;    // sum of the costs of the districts in the city
        int colorBonus = 0;        // 3 if the city has all five colours (a Haunted Quarter fills one gap)
        int abilityBonus = 0;      // sum of the scoreBonus of the districts in the city
        int completionBonus = 0;   // 4 for the first completed city, 2 for any other
        int total = 0;
        int tieBreak = 0;          // filled by ranking(); 0 when score() fills the struct
    };

    class ScoreCalculator {
    public:
        ScoreBreakdown score(const Player& player, const GameState& state) const;

        // Best first. Ties on total go to the higher entry of finalRoundRanks (indexed by seat,
        // missing counts as 0), then to the lower seat.
        std::vector<ScoreBreakdown> ranking(const GameState& state,
                                            const std::vector<int>& finalRoundRanks = {}) const;
    };
}

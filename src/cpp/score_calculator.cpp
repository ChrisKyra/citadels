#include "score_calculator.hpp"

#include <algorithm>
#include <cstddef>
#include "district_ability.hpp"

namespace citadels {

    namespace {
        // All five colours, or a Haunted Quarter standing for each colour the rest of the city misses.
        // A Quarter is one colour only, so it gives up being Special when it stands for another.
        bool hasAllColors(const City& city) {
            bool seen[5] = {false, false, false, false, false};
            int wildcards = 0;
            for (int i = 0; i < city.size(); ++i) {
                const Card& card = city.at(i);
                if (card.ability && card.ability->isScoringWildcard()) {
                    ++wildcards;
                } else {
                    seen[static_cast<int>(card.color)] = true;
                }
            }
            int missing = 0;
            for (bool present : seen) {
                if (!present) ++missing;
            }
            return missing <= wildcards;
        }
    }

    ScoreBreakdown ScoreCalculator::score(const Player& player, const GameState& state) const {
        ScoreBreakdown result;
        result.seat = player.seat;
        result.name = player.name;

        for (int i = 0; i < player.city.size(); ++i) {
            result.districtPoints += player.city.at(i).cost;
        }
        if (hasAllColors(player.city)) {
            result.colorBonus = 3;
        }
        for (int i = 0; i < player.city.size(); ++i) {
            const AbilityPtr& ability = player.city.at(i).ability;
            if (ability) result.abilityBonus += ability->scoreBonus(player, state);
        }
        if (player.city.size() >= state.districtsToFinish) {
            result.completionBonus = (state.firstToComplete == player.seat) ? 4 : 2;
        }
        result.total = result.districtPoints + result.colorBonus + result.abilityBonus + result.completionBonus;
        return result;
    }

    std::vector<ScoreBreakdown> ScoreCalculator::ranking(const GameState& state,
                                                         const std::vector<int>& finalRoundRanks) const {
        std::vector<ScoreBreakdown> result;
        for (const Player& player : state.players) {
            ScoreBreakdown entry = score(player, state);
            const std::size_t seat = static_cast<std::size_t>(player.seat);
            entry.tieBreak = seat < finalRoundRanks.size() ? finalRoundRanks[seat] : 0;
            result.push_back(std::move(entry));
        }
        std::stable_sort(result.begin(), result.end(),
                         [](const ScoreBreakdown& a, const ScoreBreakdown& b) {
                             if (a.total != b.total) return a.total > b.total;
                             if (a.tieBreak != b.tieBreak) return a.tieBreak > b.tieBreak;
                             return a.seat < b.seat;
                         });
        return result;
    }
}

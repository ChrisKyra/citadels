#include "game.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include "card_loader.hpp"
#include "character_draft.hpp"

namespace citadels {

    namespace {
        GameState setUp(const GameConfig& config, Random& rng) {
            // Also refuses player counts that GameState alone would accept (2, 3 and 7).
            CharacterDraft::create(static_cast<int>(config.playerNames.size()));

            Deck<Card> deck(CardLoader::loadDistricts(config.districtsFile));
            deck.shuffle(rng);

            std::vector<Player> players;
            for (std::size_t i = 0; i < config.playerNames.size(); ++i) {
                Player player(config.playerNames[i], static_cast<int>(i));
                player.gold = 2;
                for (int c = 0; c < 4; ++c) {
                    if (std::optional<Card> card = deck.draw()) {
                        player.hand.add(std::move(*card));
                    }
                }
                players.push_back(std::move(player));
            }
            return GameState(std::move(players), std::move(deck), config.districtsToFinish);
        }
    }

    Game::Game(const GameConfig& config, IUserInterface& ui)
        : ui_(ui),
          rng_(config.seed),
          state_(setUp(config, rng_)),
          round_(state_, ui, rng_),
          lastRoundRanks_(config.playerNames.size(), 0) {}

    std::vector<ScoreBreakdown> Game::play() {
        while (!state_.endTriggered()) {
            playRound();
        }
        return finish();
    }

    void Game::playRound() {
        round_.removeCharacters();
        round_.chooseCharacters();
        round_.callCharacters();

        // Remember the ranks now: endRound() hands the characters back.
        for (const Player& player : state_.players) {
            int best = 0;
            for (CharacterId id : player.characters) {
                best = std::max(best, state_.characters.get(id).rank());
            }
            lastRoundRanks_[static_cast<std::size_t>(player.seat)] = best;
        }

        round_.endRound();
    }

    std::vector<ScoreBreakdown> Game::finish() {
        const std::vector<ScoreBreakdown> ranking = ScoreCalculator().ranking(state_, lastRoundRanks_);

        ui_.announce("Final scoreboard:");
        int place = 1;
        for (const ScoreBreakdown& entry : ranking) {
            ui_.announce(std::to_string(place++) + ". " + entry.name + ": " + std::to_string(entry.total) +
                         " points (districts " + std::to_string(entry.districtPoints) +
                         ", colours " + std::to_string(entry.colorBonus) +
                         ", abilities " + std::to_string(entry.abilityBonus) +
                         ", completion " + std::to_string(entry.completionBonus) + ")");
        }
        ui_.announce("The winner is " + ranking.front().name);
        return ranking;
    }
}

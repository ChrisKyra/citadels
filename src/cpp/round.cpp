#include "round.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include "turn.hpp"

namespace citadels {

    Round::Round(GameState& state, IUserInterface& ui, Random& rng)
        : state_(state), ui_(ui), rng_(rng),
          draft_(CharacterDraft::create(static_cast<int>(state.players.size()))) {}

    void Round::play() {
        removeCharacters();
        chooseCharacters();
        callCharacters();
        endRound();
    }

    void Round::removeCharacters() {
        std::vector<CharacterId> ids = state_.characters.ids();
        std::shuffle(ids.begin(), ids.end(), rng_.engine());

        rs_.faceDown.push_back(ids.front());
        ids.erase(ids.begin());

        const int wanted = draft_->faceUpCount(static_cast<int>(state_.players.size()));
        for (CharacterId id : ids) {
            if (static_cast<int>(rs_.faceUp.size()) >= wanted) break;
            if (id == CharacterId::King) continue;   // the King stays in the pool
            rs_.faceUp.push_back(id);
        }

        if (rs_.faceUp.empty()) {
            ui_.announce("No characters are face up this round.");
        } else {
            std::string line = "Face-up characters:";
            for (CharacterId id : rs_.faceUp) {
                line += " " + toString(id);
            }
            ui_.announce(line);
        }
    }

    void Round::chooseCharacters() {
        draft_->run(state_, rs_, ui_);
    }

    void Round::callCharacters() {
        for (CharacterId id : state_.characters.ids()) {
            const Character& character = state_.characters.get(id);
            const std::optional<int> owner = rs_.ownerOf(id);
            if (!owner) {
                ui_.announce("The " + toString(id) + " is called, but nobody answers.");
                continue;
            }
            Player& player = state_.players[static_cast<std::size_t>(*owner)];
            if (rs_.murdered == id) {
                ui_.announce("The " + toString(id) + " (" + player.name + ") was murdered and misses their turn.");
                continue;
            }
            ui_.handOff(player);
            if (rs_.robbed == id && rs_.thiefSeat) {
                Player& thief = state_.players[static_cast<std::size_t>(*rs_.thiefSeat)];
                const int stolen = player.gold;
                thief.gold += stolen;
                player.gold = 0;
                ui_.announce(thief.name + " steals " + std::to_string(stolen) + " gold from " + player.name + ".");
            }
            Turn(state_, rs_, ui_, player, character).play();
        }
    }

    void Round::endRound() {
        if (rs_.murdered == CharacterId::King) {
            if (const std::optional<int> owner = rs_.ownerOf(CharacterId::King)) {
                state_.crownHolder = *owner;
            }
        }
        for (Player& player : state_.players) {
            player.characters.clear();
        }
        rs_ = RoundState{};
        ++state_.roundNumber;
    }
}

#include "game_state.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace citadels {

    GameState::GameState(std::vector<Player> players_, Deck<Card> districtDeck_, int districtsToFinish_)
        : players(std::move(players_)),
          characters(),
          districtDeck(std::move(districtDeck_)),
          districtsToFinish(districtsToFinish_) {
        if (players.size() < 2 || players.size() > 7) {
            throw std::invalid_argument("GameState: needs 2 to 7 players");
        }
    }

    int GameState::leftOf(int seat) const {
        if (seat < 0 || static_cast<std::size_t>(seat) >= players.size()) {
            throw std::out_of_range("GameState::leftOf: bad seat");
        }
        return (seat + 1) % static_cast<int>(players.size());
    }

    bool GameState::endTriggered() const {
        return firstToComplete.has_value();
    }
}

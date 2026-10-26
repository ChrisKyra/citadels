#pragma once

#include <optional>
#include <vector>
#include "player.hpp"
#include "card.hpp"
#include "deck.hpp"
#include "character_deck.hpp"

namespace citadels {

    // Everything that changes during a game. Move-only because CharacterDeck is.
    class GameState {
    public:
        // Throws std::invalid_argument unless there are 2 to 7 players.
        GameState(std::vector<Player> players, Deck<Card> districtDeck, int districtsToFinish = 8);

        // Seat to the left of `seat`, wrapping around.
        // Throws std::out_of_range for a bad seat.
        int leftOf(int seat) const;
        // True once some player has completed their city.
        bool endTriggered() const;

        std::vector<Player> players;
        CharacterDeck characters;
        Deck<Card> districtDeck;
        int crownHolder = 0;
        int roundNumber = 1;
        std::optional<int> firstToComplete;
        int districtsToFinish;
    };
}

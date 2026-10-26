#pragma once
#include "card.hpp"
#include "character_id.hpp"
#include "city.hpp"
#include "district_ability.hpp"
#include "hand.hpp"
#include <string>
#include <vector>

// Represents a player in the game.
// The six public data members: name, seat, gold, hand, city and characters, a vector of CharacterId
// Player(std::string name, int seat) is the constructor.

namespace citadels {
    class Player {
    public:
        std::string name;
        int seat;
        int gold;
        Hand hand;
        City city;
        std::vector<CharacterId> characters;

        Player(std::string name, int seat);

        // What the player pays for `card`: its cost minus the discount of every district in the
        // city, never below 0.
        int costOf(const Card& card) const;
        // Name rule (unless a district of the city allows duplicates) and the price: gold alone,
        // or, for a district that can be paid with cards, gold plus the other cards of the hand.
        bool canBuild(const Card& card) const;
        // Builds hand[handIndex]; payWithCards are hand indexes of cards that pay 1 gold each
        // (Thieves' Den only). Returns the paid cards in the order given. All indexes refer to the
        // hand as it was before the call. Throws std::out_of_range for a bad index and
        // std::invalid_argument for an illegal payment or build; nothing changes when it throws.
        std::vector<Card> build(int handIndex, const std::vector<int>& payWithCards = {});
    };

}
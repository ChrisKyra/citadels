#pragma once
#include "card.hpp"
#include <vector>

namespace citadels {
    class Hand {
    public:
        Hand() = default; //and the private vector of cards.
        void add(Card card); //takes the card by value.
        Card take(int index); //first read the card at that index and then delete it.
        void swapWith(Hand& other); //takes a reference, not a copy.
        int size() const;
        const Card& at(int index) const;
    private:
        std::vector<Card> cards;
    };
}
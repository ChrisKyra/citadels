#pragma once

#include <vector>
#include <algorithm>
#include <optional>
#include <utility>
#include "random.hpp"
#include <deque>

namespace citadels {

    template <typename T>
    class Deck {
    public:
        // Constructor that takes a vector of cards
        // The constructor initializes the deque with the contents of the vector
        explicit Deck(std::vector<T> cards) : cards_(std::make_move_iterator(cards.begin()), std::make_move_iterator(cards.end())) {}

        // Get the number of cards in the deck
        int size() const {
            return static_cast<int>(cards_.size());
        }

        // Shuffle the deck using the provided random number generator
        void shuffle(Random& rng) {
            std::shuffle(cards_.begin(), cards_.end(), rng.engine());
        }

        // draw() takes from the front of the deque
        std::optional<T> draw() {
            if (cards_.empty()) {
                return std::nullopt; // Return an empty optional if the deck is empty
            }
            T card = std::move(cards_.front());
            cards_.pop_front();
            return card;
        }

        // inserts a card at the back of the deck
        void putBottom(T card) {
            cards_.push_back(std::move(card));
        }

    private:
        std::deque<T> cards_;
    };

}
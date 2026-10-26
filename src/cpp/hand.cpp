#include "hand.hpp"
#include <utility>
#include <stdexcept>

namespace citadels{

    void Hand::add(Card card) {
        cards.push_back(std::move(card));
    }

    Card Hand::take(int index) {
        if (index < 0 || index >= static_cast<int>(cards.size())) {
            throw std::out_of_range("Index out of range");
        }
        
        Card card = std::move(cards[index]);
        cards.erase(cards.begin() + index);
        return card;
    }

    void Hand::swapWith(Hand& other) {
        std::swap(cards, other.cards);
    }

    int Hand::size() const {
        return cards.size();
    }

    const Card& Hand::at(int index) const {
        // Check if the index is valid
        if (index < 0 || index >= static_cast<int>(cards.size())) {
            throw std::out_of_range("Index out of range");
        }
        return cards[index];
    }
}


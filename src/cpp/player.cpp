#include "player.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "district_ability.hpp"

namespace citadels {

    namespace {
        // The name rule: no second district with the same name, unless the city has a Quarry.
        bool nameAllowed(const City& city, const Card& card) {
            if (!city.contains(card.name)) return true;
            for (int i = 0; i < city.size(); ++i) {
                const AbilityPtr& ability = city.at(i).ability;
                if (ability && ability->allowsDuplicates()) return true;
            }
            return false;
        }

        bool payableWithCards(const Card& card) {
            return card.ability && card.ability->canBePaidWithCards();
        }
    }

    Player::Player(std::string name, int seat) : name(name), seat(seat), gold(0) {}

    int Player::costOf(const Card& card) const {
        int cost = card.cost;
        for (int i = 0; i < city.size(); ++i) {
            const AbilityPtr& ability = city.at(i).ability;
            if (ability) cost -= ability->buildDiscount(card);
        }
        return std::max(cost, 0);
    }

    bool Player::canBuild(const Card& card) const {
        if (!nameAllowed(city, card)) return false;
        const int price = costOf(card);
        if (gold >= price) return true;
        if (!payableWithCards(card)) return false;

        int others = 0;
        for (int i = 0; i < hand.size(); ++i) {
            if (hand.at(i).name != card.name) ++others;
        }
        return gold + others >= price;
    }

    /*build has to look before it acts.*/
    std::vector<Card> Player::build(int handIndex, const std::vector<int>& payWithCards) {
        // 1. Every index must be inside the hand.
        if (handIndex < 0 || handIndex >= hand.size()) {
            throw std::out_of_range("Hand index is out of range");
        }
        for (int index : payWithCards) {
            if (index < 0 || index >= hand.size()) {
                throw std::out_of_range("Payment index is out of range");
            }
        }

        // 2. The payment and the build itself must be legal.
        const Card card = hand.at(handIndex);
        std::vector<int> sorted = payWithCards;
        std::sort(sorted.begin(), sorted.end());
        if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) {
            throw std::invalid_argument("The same card cannot pay twice");
        }
        if (std::find(sorted.begin(), sorted.end(), handIndex) != sorted.end()) {
            throw std::invalid_argument("A district cannot pay for itself");
        }
        if (!sorted.empty() && !payableWithCards(card)) {
            throw std::invalid_argument("This district cannot be paid with cards");
        }
        const int price = costOf(card);
        const int cardsPaid = static_cast<int>(sorted.size());
        if (cardsPaid > price) {
            throw std::invalid_argument("More cards than the price");
        }
        if (gold < price - cardsPaid) {
            throw std::invalid_argument("Not enough gold to build this card");
        }
        if (!nameAllowed(city, card)) {
            throw std::invalid_argument("This district is already in the city");
        }

        // Nothing can fail from here on.
        std::vector<Card> paid;
        for (int index : payWithCards) {
            paid.push_back(hand.at(index));
        }
        gold -= price - cardsPaid;
        city.add(card);
        sorted.push_back(handIndex);
        std::sort(sorted.begin(), sorted.end(), std::greater<int>());   // remove from the back
        for (int index : sorted) {
            hand.take(index);
        }
        return paid;
    }

}

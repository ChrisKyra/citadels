#pragma once

#include <string>
#include <vector>
#include "card.hpp"

namespace citadels{

    class City {
    public:
        City() = default;
        void add(Card card);
        int size() const;
        const Card& at(int index) const;
        bool contains(const std::string& name) const;
        int countColor(Color c) const;
        bool hasAllColors() const;
        Card remove(int index);
        bool isComplete(int target) const;
    private:
        std::vector<Card> districts;
    };

}
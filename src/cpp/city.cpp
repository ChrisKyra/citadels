#include "city.hpp"
#include <stdexcept>
#include <utility>

namespace citadels {

    void City::add(Card card) {
        districts.push_back(std::move(card));
    }

    int City::size() const {
        return districts.size();
    }

    const Card& City::at(int index) const {
        if (index < 0 || index >= static_cast<int>(districts.size())) {
            throw std::out_of_range("Index out of range");
        }
        return districts[index];
    }

    bool City::contains(const std::string& name) const {
        for (const auto& district : districts) {
            if (district.name == name) {
                return true;
            }
        }
        return false;
    }

    int City::countColor(Color c) const {
        int count = 0;
        for (const auto& district : districts) {
            if (district.color == c) {
                count++;
            }
        }
        return count;
    }

    bool City::hasAllColors() const {
        bool hasNoble = false, hasTrade = false, hasReligious = false, hasMilitary = false, hasSpecial = false;
        for (const auto& district : districts) {
            switch (district.color) {
                case Color::Noble: hasNoble = true; break;
                case Color::Trade: hasTrade = true; break;
                case Color::Religious: hasReligious = true; break;
                case Color::Military: hasMilitary = true; break;
                case Color::Special: hasSpecial = true; break; 
            }
        }
        return hasNoble && hasTrade && hasReligious && hasMilitary && hasSpecial;
    }

    Card City::remove(int index) {
        if (index < 0 || index >= static_cast<int>(districts.size())) {
            throw std::out_of_range("Index out of range");
        }
        
        Card card = std::move(districts[index]);
        districts.erase(districts.begin() + index);
        return card;
    }

    bool City::isComplete(int target) const {
        return size() >= target;
    }

}
#pragma once
#include <memory>
#include <string>
#include <utility>
#include "color.hpp"

namespace citadels {

class DistrictAbility;

struct Card {
    std::string name;
    int cost;
    Color color;
    std::shared_ptr<const DistrictAbility> ability;   // AbilityPtr; null for a district without one

    Card(const std::string& name, int cost, Color color,
         std::shared_ptr<const DistrictAbility> ability = nullptr)
        : name(name), cost(cost), color(color), ability(std::move(ability)) {}
};
}

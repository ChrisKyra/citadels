#include "ability_factory.hpp"

#include <stdexcept>

namespace citadels {

    AbilityPtr AbilityFactory::create(const std::string& id) {
        if (id.empty())                return nullptr;
        if (id == "dragon_gate")       return std::make_shared<DragonGate>();
        if (id == "factory")           return std::make_shared<Factory>();
        if (id == "haunted_quarter")   return std::make_shared<HauntedQuarter>();
        if (id == "imperial_treasury") return std::make_shared<ImperialTreasury>();
        if (id == "keep")              return std::make_shared<Keep>();
        if (id == "laboratory")        return std::make_shared<Laboratory>();
        if (id == "library")           return std::make_shared<Library>();
        if (id == "map_room")          return std::make_shared<MapRoom>();
        if (id == "quarry")            return std::make_shared<Quarry>();
        if (id == "school_of_magic")   return std::make_shared<SchoolOfMagic>();
        if (id == "smithy")            return std::make_shared<Smithy>();
        if (id == "statue")            return std::make_shared<Statue>();
        if (id == "thieves_den")       return std::make_shared<ThievesDen>();
        if (id == "wishing_well")      return std::make_shared<WishingWell>();
        throw std::invalid_argument("AbilityFactory::create: unknown ability id '" + id + "'");
    }
}

#include "character_factory.hpp"

#include <stdexcept>
#include "characters.hpp"

namespace citadels {

    CharacterPtr CharacterFactory::create(CharacterId id) {
        switch (id) {
            case CharacterId::Assassin:  return std::make_unique<Assassin>();
            case CharacterId::Thief:     return std::make_unique<Thief>();
            case CharacterId::Magician:  return std::make_unique<Magician>();
            case CharacterId::King:      return std::make_unique<King>();
            case CharacterId::Bishop:    return std::make_unique<Bishop>();
            case CharacterId::Merchant:  return std::make_unique<Merchant>();
            case CharacterId::Architect: return std::make_unique<Architect>();
            case CharacterId::Warlord:   return std::make_unique<Warlord>();
        }
        throw std::invalid_argument("CharacterFactory::create: unknown character id");
    }

}

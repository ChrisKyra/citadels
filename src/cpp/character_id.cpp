#include "character_id.hpp"
#include <stdexcept>

namespace citadels {

    std::string toString(CharacterId id) {
        switch (id) {
            case CharacterId::Assassin:  return "Assassin";
            case CharacterId::Thief:     return "Thief";
            case CharacterId::Magician:  return "Magician";
            case CharacterId::King:      return "King";
            case CharacterId::Bishop:    return "Bishop";
            case CharacterId::Merchant:  return "Merchant";
            case CharacterId::Architect: return "Architect";
            case CharacterId::Warlord:   return "Warlord";
        }
        throw std::invalid_argument("toString: unknown character id");
    }
}

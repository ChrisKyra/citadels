//It is an enum class with the eight base-game characters: Assassin, Thief, Magician, King, Bishop, Merchant, Architect, Warlord.
//It carries no numbers for now.

#pragma once
#include <string>

namespace citadels {
    enum class CharacterId {
        Assassin,
        Thief,
        Magician,
        King,
        Bishop,
        Merchant,
        Architect,
        Warlord
    };

    std::string toString(CharacterId id);
}

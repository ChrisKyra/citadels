#pragma once

#include "characters.hpp"
#include "character_id.hpp"

namespace citadels {

    class CharacterFactory {
    public:
        // Throws std::invalid_argument for an id outside the eight.
        static CharacterPtr create(CharacterId id);
    };

}

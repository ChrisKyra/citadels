#pragma once

#include <vector>
#include "characters.hpp"
#include "character_id.hpp"

namespace citadels {

    // Owns the pool of characters in play. Move-only because of the unique_ptrs.
    class CharacterDeck {
    public:
        // All eight characters, in rank order.
        CharacterDeck();
        // Just these characters, in this order.
        // Throws std::invalid_argument if an id appears twice.
        explicit CharacterDeck(const std::vector<CharacterId>& wanted);

        CharacterDeck(const CharacterDeck&) = delete;
        CharacterDeck& operator=(const CharacterDeck&) = delete;
        CharacterDeck(CharacterDeck&&) = default;
        CharacterDeck& operator=(CharacterDeck&&) = default;

        // Ids in pool order (a copy).
        std::vector<CharacterId> ids() const;
        // Throws std::out_of_range if the character is not in the pool.
        const Character& get(CharacterId id) const;

    private:
        std::vector<CharacterPtr> pool;
    };

}

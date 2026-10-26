#include "character_deck.hpp"

#include <stdexcept>
#include "character_factory.hpp"

namespace citadels {

    CharacterDeck::CharacterDeck()
        : CharacterDeck(std::vector<CharacterId>{
              CharacterId::Assassin, CharacterId::Thief,    CharacterId::Magician, CharacterId::King,
              CharacterId::Bishop,   CharacterId::Merchant, CharacterId::Architect, CharacterId::Warlord}) {}

    CharacterDeck::CharacterDeck(const std::vector<CharacterId>& wanted) {
        for (CharacterId id : wanted) {
            for (const CharacterPtr& existing : pool) {
                if (existing->id() == id) {
                    throw std::invalid_argument("CharacterDeck: duplicate character id");
                }
            }
            pool.push_back(CharacterFactory::create(id));
        }
    }

    std::vector<CharacterId> CharacterDeck::ids() const {
        std::vector<CharacterId> result;
        for (const CharacterPtr& ch : pool) {
            result.push_back(ch->id());
        }
        return result;
    }

    const Character& CharacterDeck::get(CharacterId id) const {
        for (const CharacterPtr& ch : pool) {
            if (ch->id() == id) {
                return *ch;
            }
        }
        throw std::out_of_range("CharacterDeck::get: character not in the pool");
    }

}

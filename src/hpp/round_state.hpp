#pragma once

#include <array>
#include <optional>
#include <stdexcept>
#include <vector>
#include "character_id.hpp"

namespace citadels {

    // What happened during one round. A default-constructed RoundState means nothing has happened yet.
    struct RoundState {
        std::vector<CharacterId> faceDown, faceUp;
        std::optional<CharacterId> murdered, robbed;
        std::optional<int> thiefSeat;

        // Seat that chose this character, or empty.
        std::optional<int> ownerOf(CharacterId id) const {
            const int seat = owners_[static_cast<std::size_t>(id)];
            if (seat < 0) return std::nullopt;
            return seat;
        }

        // Throws std::invalid_argument if the character already has an owner or the seat is negative.
        void assign(CharacterId id, int seat) {
            if (seat < 0) {
                throw std::invalid_argument("RoundState::assign: negative seat");
            }
            int& owner = owners_[static_cast<std::size_t>(id)];
            if (owner >= 0) {
                throw std::invalid_argument("RoundState::assign: character already has an owner");
            }
            owner = seat;
        }

    private:
        // Indexed by CharacterId, which is declared in rank order; -1 means unowned.
        std::array<int, 8> owners_ = {-1, -1, -1, -1, -1, -1, -1, -1};
    };
}

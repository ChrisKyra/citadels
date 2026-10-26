#pragma once

#include <memory>
#include <vector>
#include "game_state.hpp"
#include "round_state.hpp"
#include "user_interface.hpp"

namespace citadels {

    // How characters are picked at the start of a round. The caller fills rs.faceDown and
    // rs.faceUp before run().
    class CharacterDraft {
    public:
        virtual ~CharacterDraft() = default;

        // Throws std::invalid_argument for a player count this draft does not handle.
        virtual int faceUpCount(int players) const = 0;
        virtual void run(GameState& state, RoundState& rs, IUserInterface& ui) = 0;

        // 2, 3, 4 to 6 and 7 players each get their own draft.
        // Throws std::invalid_argument for any other count.
        static std::unique_ptr<CharacterDraft> create(int players);

    protected:
        // The pool in rank order, minus rs.faceDown and rs.faceUp.
        static std::vector<CharacterId> availableCharacters(const GameState& state, const RoundState& rs);
        // handOff(player), then the player chooses from `options`; the pick is assigned to the
        // player, removed from `options` and returned.
        static CharacterId pick(GameState& state, RoundState& rs, IUserInterface& ui,
                                std::vector<CharacterId>& options, int seat);
        // `count` players pick in turn, starting at `seat` and moving left; `seat` ends up on the next one.
        static void pickInTurn(GameState& state, RoundState& rs, IUserInterface& ui,
                               std::vector<CharacterId>& options, int& seat, int count);
        // The player chooses a card from `options` to put face-down (appended to rs.faceDown).
        // No hand-off: it is made right after the same player's pick.
        static void discard(GameState& state, RoundState& rs, IUserInterface& ui,
                            std::vector<CharacterId>& options, int seat);
    };

    using DraftPtr = std::unique_ptr<CharacterDraft>;

    // The 4 to 6 player draft: pick in seat order from the crown holder, one card is left over.
    class StandardDraft : public CharacterDraft {
    public:
        int faceUpCount(int players) const override;
        void run(GameState& state, RoundState& rs, IUserInterface& ui) override;
    };

    // Everybody plays two characters; the players alternate picking and discarding face-down.
    class TwoPlayerDraft final : public CharacterDraft {
    public:
        int faceUpCount(int players) const override;
        void run(GameState& state, RoundState& rs, IUserInterface& ui) override;
    };

    // Everybody plays two characters; two trips around the table.
    class ThreePlayerDraft final : public CharacterDraft {
    public:
        int faceUpCount(int players) const override;
        void run(GameState& state, RoundState& rs, IUserInterface& ui) override;
    };

    // Like the standard draft, but the last player chooses between the card left over and the face-down one.
    class SevenPlayerDraft final : public StandardDraft {
    public:
        int faceUpCount(int players) const override;
        void run(GameState& state, RoundState& rs, IUserInterface& ui) override;
    };
}

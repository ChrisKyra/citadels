#include "character_draft.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace citadels {

    namespace {
        bool contains(const std::vector<CharacterId>& ids, CharacterId id) {
            return std::find(ids.begin(), ids.end(), id) != ids.end();
        }

        std::vector<std::string> namesOf(const std::vector<CharacterId>& ids) {
            std::vector<std::string> names;
            for (CharacterId id : ids) {
                names.push_back(toString(id));
            }
            return names;
        }
    }

    DraftPtr CharacterDraft::create(int players) {
        switch (players) {
            case 2: return std::make_unique<TwoPlayerDraft>();
            case 3: return std::make_unique<ThreePlayerDraft>();
            case 4:
            case 5:
            case 6: return std::make_unique<StandardDraft>();
            case 7: return std::make_unique<SevenPlayerDraft>();
            default: throw std::invalid_argument("CharacterDraft::create: unsupported player count");
        }
    }

    std::vector<CharacterId> CharacterDraft::availableCharacters(const GameState& state, const RoundState& rs) {
        std::vector<CharacterId> available;
        for (CharacterId id : state.characters.ids()) {
            if (!contains(rs.faceDown, id) && !contains(rs.faceUp, id)) {
                available.push_back(id);
            }
        }
        return available;
    }

    CharacterId CharacterDraft::pick(GameState& state, RoundState& rs, IUserInterface& ui,
                                     std::vector<CharacterId>& options, int seat) {
        Player& player = state.players[static_cast<std::size_t>(seat)];
        ui.handOff(player);

        const int index = ui.choose(player.name + ", choose your character:", namesOf(options));
        const CharacterId chosen = options[static_cast<std::size_t>(index)];

        rs.assign(chosen, seat);
        player.characters.push_back(chosen);
        options.erase(options.begin() + index);
        return chosen;
    }

    void CharacterDraft::discard(GameState& state, RoundState& rs, IUserInterface& ui,
                                 std::vector<CharacterId>& options, int seat) {
        const Player& player = state.players[static_cast<std::size_t>(seat)];
        const int index = ui.choose(player.name + ", discard a character face-down:", namesOf(options));
        rs.faceDown.push_back(options[static_cast<std::size_t>(index)]);
        options.erase(options.begin() + index);
    }

    void CharacterDraft::pickInTurn(GameState& state, RoundState& rs, IUserInterface& ui,
                                   std::vector<CharacterId>& options, int& seat, int count) {
        for (int i = 0; i < count; ++i) {
            pick(state, rs, ui, options, seat);
            seat = state.leftOf(seat);
        }
    }

    // ---- 4 to 6 players ----

    int StandardDraft::faceUpCount(int players) const {
        switch (players) {
            case 4: return 2;
            case 5: return 1;
            case 6: return 0;
            default: throw std::invalid_argument("StandardDraft: unsupported player count");
        }
    }

    void StandardDraft::run(GameState& state, RoundState& rs, IUserInterface& ui) {
        std::vector<CharacterId> available = availableCharacters(state, rs);
        int seat = state.crownHolder;
        pickInTurn(state, rs, ui, available, seat, static_cast<int>(state.players.size()));

        for (CharacterId id : available) {
            rs.faceDown.push_back(id);
        }
    }

    // ---- 2 players ----

    int TwoPlayerDraft::faceUpCount(int players) const {
        if (players != 2) throw std::invalid_argument("TwoPlayerDraft: needs exactly 2 players");
        return 0;
    }

    void TwoPlayerDraft::run(GameState& state, RoundState& rs, IUserInterface& ui) {
        std::vector<CharacterId> available = availableCharacters(state, rs);
        const int a = state.crownHolder;
        const int b = state.leftOf(a);

        pick(state, rs, ui, available, a);
        pick(state, rs, ui, available, b);
        discard(state, rs, ui, available, b);
        pick(state, rs, ui, available, a);
        discard(state, rs, ui, available, a);
        pick(state, rs, ui, available, b);

        for (CharacterId id : available) {
            rs.faceDown.push_back(id);
        }
    }

    // ---- 3 players ----

    int ThreePlayerDraft::faceUpCount(int players) const {
        if (players != 3) throw std::invalid_argument("ThreePlayerDraft: needs exactly 3 players");
        return 0;
    }

    void ThreePlayerDraft::run(GameState& state, RoundState& rs, IUserInterface& ui) {
        std::vector<CharacterId> available = availableCharacters(state, rs);
        int seat = state.crownHolder;
        for (int lap = 0; lap < 2; ++lap) {
            pickInTurn(state, rs, ui, available, seat, 3);
        }
        for (CharacterId id : available) {
            rs.faceDown.push_back(id);
        }
    }

    // ---- 7 players ----

    int SevenPlayerDraft::faceUpCount(int players) const {
        if (players != 7) throw std::invalid_argument("SevenPlayerDraft: needs exactly 7 players");
        return 0;
    }

    void SevenPlayerDraft::run(GameState& state, RoundState& rs, IUserInterface& ui) {
        std::vector<CharacterId> available = availableCharacters(state, rs);
        int seat = state.crownHolder;
        pickInTurn(state, rs, ui, available, seat, static_cast<int>(state.players.size()) - 1);

        // The last player chooses between what is left and the face-down card(s), in rank order.
        std::vector<CharacterId> choices = available;
        choices.insert(choices.end(), rs.faceDown.begin(), rs.faceDown.end());
        std::sort(choices.begin(), choices.end());

        pick(state, rs, ui, choices, seat);
        rs.faceDown = choices;
    }
}

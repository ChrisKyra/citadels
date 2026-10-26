#include "characters.hpp"
#include "turn_context.hpp"
#include "district_ability.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace citadels {

    namespace {
        // The pool's ids in rank order, minus the excluded ones.
        std::vector<CharacterId> candidatesFor(const TurnContext& ctx, const std::vector<CharacterId>& excluded) {
            std::vector<CharacterId> result;
            for (CharacterId id : ctx.state.characters.ids()) {
                if (std::find(excluded.begin(), excluded.end(), id) == excluded.end()) {
                    result.push_back(id);
                }
            }
            return result;
        }

        std::vector<std::string> namesOf(const std::vector<CharacterId>& ids) {
            std::vector<std::string> names;
            for (CharacterId id : ids) {
                names.push_back(toString(id));
            }
            return names;
        }

        std::string describe(const Card& card) {
            return card.name + " (" + std::to_string(card.cost) + ")";
        }

        void drawCards(TurnContext& ctx, int count) {
            for (int i = 0; i < count; ++i) {
                std::optional<Card> card = ctx.state.districtDeck.draw();
                if (!card) return;
                ctx.player.hand.add(std::move(*card));
            }
        }
    }

    void Character::collectIncome(TurnContext& ctx) const {
        if (ctx.incomeTaken) return;
        const std::optional<Color> color = incomeColor();
        if (!color) return;
        int amount = ctx.player.city.countColor(*color);
        // A School of Magic counts as one more district of the character's colour.
        for (int i = 0; i < ctx.player.city.size(); ++i) {
            const Card& district = ctx.player.city.at(i);
            if (district.ability && district.ability->isIncomeWildcard() && district.color != *color) {
                ++amount;
            }
        }
        ctx.player.gold += amount;
        ctx.incomeTaken = true;
        if (amount > 0) {
            ctx.ui.announce(ctx.player.name + " collects " + std::to_string(amount) +
                            " gold of income as the " + name() + ".");
        }
    }


    CharacterId Assassin::id() const { return CharacterId::Assassin; }
    std::string Assassin::name() const { return "Assassin"; }
    int Assassin::rank() const { return 1; }
    bool Assassin::hasPower() const { return true; }

    CharacterId Thief::id() const { return CharacterId::Thief; }
    std::string Thief::name() const { return "Thief"; }
    int Thief::rank() const { return 2; }
    bool Thief::hasPower() const { return true; }

    CharacterId Magician::id() const { return CharacterId::Magician; }
    std::string Magician::name() const { return "Magician"; }
    int Magician::rank() const { return 3; }
    bool Magician::hasPower() const { return true; }

    CharacterId King::id() const { return CharacterId::King; }
    std::string King::name() const { return "King"; }
    int King::rank() const { return 4; }
    std::optional<Color> King::incomeColor() const { return Color::Noble; }

    CharacterId Bishop::id() const { return CharacterId::Bishop; }
    std::string Bishop::name() const { return "Bishop"; }
    int Bishop::rank() const { return 5; }
    std::optional<Color> Bishop::incomeColor() const { return Color::Religious; }
    bool Bishop::protectsCity() const { return true; }

    CharacterId Merchant::id() const { return CharacterId::Merchant; }
    std::string Merchant::name() const { return "Merchant"; }
    int Merchant::rank() const { return 6; }
    std::optional<Color> Merchant::incomeColor() const { return Color::Trade; }

    CharacterId Architect::id() const { return CharacterId::Architect; }
    std::string Architect::name() const { return "Architect"; }
    int Architect::rank() const { return 7; }
    int Architect::buildLimit() const { return 3; }

    CharacterId Warlord::id() const { return CharacterId::Warlord; }
    std::string Warlord::name() const { return "Warlord"; }
    int Warlord::rank() const { return 8; }
    std::optional<Color> Warlord::incomeColor() const { return Color::Military; }

    void Merchant::afterAction(TurnContext& ctx) const {
        ctx.player.gold += 1;
        ctx.ui.announce(ctx.player.name + " receives 1 extra gold as the Merchant.");
    }

    void Architect::afterAction(TurnContext& ctx) const {
        drawCards(ctx, 2);
        ctx.ui.announce(ctx.player.name + " draws extra cards as the Architect.");
    }

    void King::onReveal(TurnContext& ctx) const {
        ctx.state.crownHolder = ctx.player.seat;
        ctx.ui.announce(ctx.player.name + " takes the crown.");
    }

    void Assassin::usePower(TurnContext& ctx) const {
        if (ctx.powerUsed) return;
        const std::vector<CharacterId> targets = candidatesFor(ctx, {CharacterId::Assassin});
        const int pick = ctx.ui.choose("Which character do you murder?", namesOf(targets));
        ctx.round.murdered = targets[static_cast<std::size_t>(pick)];
        ctx.powerUsed = true;
        ctx.ui.announce(ctx.player.name + " murders the " + toString(*ctx.round.murdered) + ".");
    }

    void Thief::usePower(TurnContext& ctx) const {
        if (ctx.powerUsed) return;
        std::vector<CharacterId> excluded = {CharacterId::Assassin, CharacterId::Thief};
        if (ctx.round.murdered) excluded.push_back(*ctx.round.murdered);
        const std::vector<CharacterId> targets = candidatesFor(ctx, excluded);
        const int pick = ctx.ui.choose("Which character do you rob?", namesOf(targets));
        ctx.round.robbed = targets[static_cast<std::size_t>(pick)];
        ctx.round.thiefSeat = ctx.player.seat;
        ctx.powerUsed = true;
        ctx.ui.announce(ctx.player.name + " robs the " + toString(*ctx.round.robbed) + ".");
    }

    void Magician::usePower(TurnContext& ctx) const {
        if (ctx.powerUsed) return;
        Player& player = ctx.player;
        const int mode = ctx.ui.choose("Magician power:", {"Swap hands with a player",
                                                          "Discard cards and draw the same number"});
        if (mode == 0) {
            std::vector<std::string> names;
            std::vector<std::size_t> seats;
            for (std::size_t i = 0; i < ctx.state.players.size(); ++i) {
                if (&ctx.state.players[i] == &player) continue;
                names.push_back(ctx.state.players[i].name);
                seats.push_back(i);
            }
            const int pick = ctx.ui.choose("Swap hands with whom?", names);
            Player& other = ctx.state.players[seats[static_cast<std::size_t>(pick)]];
            player.hand.swapWith(other.hand);
            ctx.ui.announce(player.name + " swaps hands with " + other.name + ".");
        } else {
            std::vector<Card> discarded;
            while (player.hand.size() > 0) {
                std::vector<std::string> options;
                for (int i = 0; i < player.hand.size(); ++i) {
                    options.push_back(describe(player.hand.at(i)));
                }
                options.push_back("Done");
                const int pick = ctx.ui.choose("Discard which card?", options);
                if (pick == player.hand.size()) break;
                discarded.push_back(player.hand.take(pick));
            }
            const int count = static_cast<int>(discarded.size());
            for (Card& card : discarded) {
                ctx.state.districtDeck.putBottom(std::move(card));
            }
            drawCards(ctx, count);
            ctx.ui.announce(player.name + " discards " + std::to_string(count) + " cards and draws new ones.");
        }
        ctx.powerUsed = true;
    }

    void Warlord::atEndOfTurn(TurnContext& ctx) const {
        // Cities of players who own a protecting character this round (even a murdered one).
        std::vector<int> protectedSeats;
        for (CharacterId id : ctx.state.characters.ids()) {
            if (!ctx.state.characters.get(id).protectsCity()) continue;
            if (std::optional<int> owner = ctx.round.ownerOf(id)) {
                protectedSeats.push_back(*owner);
            }
        }

        struct Target { std::size_t player; int district; };
        std::vector<Target> targets;
        std::vector<std::string> options;
        for (std::size_t p = 0; p < ctx.state.players.size(); ++p) {
            const Player& owner = ctx.state.players[p];
            if (owner.city.size() >= ctx.state.districtsToFinish) continue;
            if (std::find(protectedSeats.begin(), protectedSeats.end(), owner.seat) != protectedSeats.end()) continue;
            for (int d = 0; d < owner.city.size(); ++d) {
                const Card& card = owner.city.at(d);
                if (card.ability && !card.ability->canBeDestroyed()) continue;   // the Keep
                if (ctx.player.gold < card.cost - 1) continue;
                targets.push_back({p, d});
                options.push_back(owner.name + ": " + card.name + " (pay " + std::to_string(card.cost - 1) + ")");
            }
        }
        if (targets.empty()) return;

        options.push_back("Don't destroy");
        const int pick = ctx.ui.choose("Destroy a district?", options);
        if (static_cast<std::size_t>(pick) >= targets.size()) return;

        const Target& target = targets[static_cast<std::size_t>(pick)];
        Player& owner = ctx.state.players[target.player];
        Card destroyed = owner.city.remove(target.district);
        ctx.player.gold -= destroyed.cost - 1;
        ctx.ui.announce(ctx.player.name + " destroys " + owner.name + "'s " + destroyed.name + ".");
    }

}

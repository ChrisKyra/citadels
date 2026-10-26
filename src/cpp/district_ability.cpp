#include "district_ability.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include "card.hpp"
#include "game_state.hpp"
#include "player.hpp"
#include "turn_context.hpp"

namespace citadels {

    namespace {
        bool wasUsed(const TurnContext& ctx, const std::string& id) {
            return std::find(ctx.abilitiesUsed.begin(), ctx.abilitiesUsed.end(), id) != ctx.abilitiesUsed.end();
        }

        void drawInto(TurnContext& ctx, int count) {
            for (int i = 0; i < count; ++i) {
                std::optional<Card> card = ctx.state.districtDeck.draw();
                if (!card) return;
                ctx.player.hand.add(std::move(*card));
            }
        }
    }

    // ---- Dragon Gate ----
    std::string DragonGate::id() const { return "dragon_gate"; }
    std::string DragonGate::description() const { return "Worth 2 extra points at the end of the game."; }
    int DragonGate::scoreBonus(const Player&, const GameState&) const { return 2; }

    // ---- Factory ----
    std::string Factory::id() const { return "factory"; }
    std::string Factory::description() const { return "Other special districts cost you 1 gold less to build."; }
    int Factory::buildDiscount(const Card& target) const {
        const bool isFactory = target.ability && target.ability->id() == id();
        return target.color == Color::Special && !isFactory ? 1 : 0;
    }

    // ---- Haunted Quarter ----
    std::string HauntedQuarter::id() const { return "haunted_quarter"; }
    std::string HauntedQuarter::description() const { return "For the colour bonus it counts as a district of any one colour."; }
    bool HauntedQuarter::isScoringWildcard() const { return true; }

    // ---- Imperial Treasury ----
    std::string ImperialTreasury::id() const { return "imperial_treasury"; }
    std::string ImperialTreasury::description() const { return "At the end of the game, 1 extra point per gold you hold."; }
    int ImperialTreasury::scoreBonus(const Player& owner, const GameState&) const { return owner.gold; }

    // ---- Keep ----
    std::string Keep::id() const { return "keep"; }
    std::string Keep::description() const { return "The Warlord cannot destroy it."; }
    bool Keep::canBeDestroyed() const { return false; }

    // ---- Laboratory ----
    std::string Laboratory::id() const { return "laboratory"; }
    std::string Laboratory::description() const { return "Once per turn, discard a card from your hand to gain 2 gold."; }

    bool Laboratory::canUse(const TurnContext& ctx) const {
        return ctx.player.hand.size() > 0 && !wasUsed(ctx, id());
    }

    void Laboratory::use(TurnContext& ctx) const {
        Player& player = ctx.player;
        std::vector<std::string> options;
        for (int i = 0; i < player.hand.size(); ++i) {
            const Card& card = player.hand.at(i);
            options.push_back(card.name + " (" + std::to_string(card.cost) + ")");
        }
        options.push_back("Back");

        const int pick = ctx.ui.choose("Discard which card?", options);
        if (pick >= player.hand.size()) return;

        ctx.state.districtDeck.putBottom(player.hand.take(pick));
        player.gold += 2;
        ctx.abilitiesUsed.push_back(id());
        ctx.ui.announce(player.name + " discards a card in the Laboratory and gains 2 gold.");
    }

    // ---- Library ----
    std::string Library::id() const { return "library"; }
    std::string Library::description() const { return "When you draw cards, you keep both of them."; }
    bool Library::keepsBothDrawnCards() const { return true; }

    // ---- Map Room ----
    std::string MapRoom::id() const { return "map_room"; }
    std::string MapRoom::description() const { return "At the end of the game, 1 extra point per card in your hand."; }
    int MapRoom::scoreBonus(const Player& owner, const GameState&) const { return owner.hand.size(); }

    // ---- Quarry ----
    std::string Quarry::id() const { return "quarry"; }
    std::string Quarry::description() const { return "You may build districts that are already in your city."; }
    bool Quarry::allowsDuplicates() const { return true; }

    // ---- School of Magic ----
    std::string SchoolOfMagic::id() const { return "school_of_magic"; }
    std::string SchoolOfMagic::description() const { return "For income it counts as a district of the colour of your character."; }
    bool SchoolOfMagic::isIncomeWildcard() const { return true; }

    // ---- Smithy ----
    std::string Smithy::id() const { return "smithy"; }
    std::string Smithy::description() const { return "Once per turn, pay 2 gold to draw 3 cards."; }

    bool Smithy::canUse(const TurnContext& ctx) const {
        return ctx.player.gold >= 2 && !wasUsed(ctx, id());
    }

    void Smithy::use(TurnContext& ctx) const {
        ctx.player.gold -= 2;
        drawInto(ctx, 3);
        ctx.abilitiesUsed.push_back(id());
        ctx.ui.announce(ctx.player.name + " pays 2 gold at the Smithy and draws cards.");
    }

    // ---- Statue ----
    std::string Statue::id() const { return "statue"; }
    std::string Statue::description() const { return "Worth 5 extra points at the end of the game if you hold the crown."; }
    int Statue::scoreBonus(const Player& owner, const GameState& state) const {
        return state.crownHolder == owner.seat ? 5 : 0;
    }

    // ---- Thieves' Den ----
    std::string ThievesDen::id() const { return "thieves_den"; }
    std::string ThievesDen::description() const { return "You may pay for it with cards from your hand instead of gold, 1 gold per card."; }
    bool ThievesDen::canBePaidWithCards() const { return true; }

    // ---- Wishing Well ----
    std::string WishingWell::id() const { return "wishing_well"; }
    std::string WishingWell::description() const { return "At the end of the game, 1 extra point per special district in your city (itself included)."; }
    int WishingWell::scoreBonus(const Player& owner, const GameState&) const {
        int count = 0;
        for (int i = 0; i < owner.city.size(); ++i) {
            if (owner.city.at(i).color == Color::Special) ++count;
        }
        return count;
    }
}

#include "turn.hpp"
#include "district_ability.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace citadels {

    namespace {
        std::string describe(const Card& card) {
            return card.name + " (" + std::to_string(card.cost) + ")";
        }

        bool hasAbilityDistrict(const Player& player) {
            for (int i = 0; i < player.city.size(); ++i) {
                if (player.city.at(i).ability) return true;
            }
            for (int i = 0; i < player.hand.size(); ++i) {
                if (player.hand.at(i).ability) return true;
            }
            return false;
        }

        // True if a district in the player's city keeps both cards of "Draw 2 cards" (Library).
        bool keepsBothDrawnCards(const Player& player) {
            for (int i = 0; i < player.city.size(); ++i) {
                const AbilityPtr& ability = player.city.at(i).ability;
                if (ability && ability->keepsBothDrawnCards()) return true;
            }
            return false;
        }
    }

    Turn::Turn(GameState& state, RoundState& round, IUserInterface& ui, Player& player, const Character& character)
        : ctx_(state, round, ui, player, character) {}

    void Turn::onReveal() {
        ctx_.character.onReveal(ctx_);
    }

    void Turn::play() {
        ctx_.ui.announce(ctx_.player.name + " plays the " + ctx_.character.name() + ".");
        onReveal();
        ctx_.ui.showTable(ctx_.state);
        ctx_.ui.showPrivate(ctx_.player);
        takeAction();
        mainPhase();
        endOfTurn();
    }

    void Turn::takeAction() {
        Player& player = ctx_.player;
        IUserInterface& ui = ctx_.ui;

        const int pick = ui.choose("Choose an action:", {"Take 2 gold", "Draw 2 cards"});
        if (pick == 0) {
            player.gold += 2;
            ui.announce(player.name + " takes 2 gold.");
        } else {
            std::vector<Card> drawn;
            for (int i = 0; i < 2; ++i) {
                if (std::optional<Card> card = ctx_.state.districtDeck.draw()) {
                    drawn.push_back(std::move(*card));
                }
            }
            if (drawn.size() == 2 && keepsBothDrawnCards(player)) {
                player.hand.add(drawn[0]);
                player.hand.add(drawn[1]);
                ui.announce(player.name + " draws 2 cards and keeps both.");
            } else if (drawn.size() == 2) {
                const int keep = ui.choose("Keep one card:", {describe(drawn[0]), describe(drawn[1])});
                const std::size_t kept = static_cast<std::size_t>(keep);
                player.hand.add(drawn[kept]);
                ctx_.state.districtDeck.putBottom(drawn[1 - kept]);
                ui.announce(player.name + " draws 2 cards and keeps " + drawn[kept].name + ".");
            } else if (drawn.size() == 1) {
                player.hand.add(drawn[0]);
                ui.announce(player.name + " draws 1 card.");
            } else {
                ui.announce("The district deck is empty.");
            }
        }
        ctx_.actionTaken = true;
        ctx_.character.afterAction(ctx_);
    }

    void Turn::mainPhase() {
        Player& player = ctx_.player;
        IUserInterface& ui = ctx_.ui;

        while (true) {
            ui.showPrivate(player);
            // Hand indexes of cards that can be built right now.
            std::vector<int> buildable;
            if (ctx_.buildsLeft > 0) {
                for (int i = 0; i < player.hand.size(); ++i) {
                    if (player.canBuild(player.hand.at(i))) {
                        buildable.push_back(i);
                    }
                }
            }

            enum class Entry { Build, Power, District, Income, Help, End };
            std::vector<std::string> options;
            std::vector<Entry> entries;
            std::vector<int> cityIndexes;   // city index behind each Entry::District, in menu order
            if (!buildable.empty()) {
                options.push_back("Build a district");
                entries.push_back(Entry::Build);
            }
            if (ctx_.character.hasPower() && !ctx_.powerUsed) {
                options.push_back("Use power");
                entries.push_back(Entry::Power);
            }
            for (int i = 0; i < player.city.size(); ++i) {
                const Card& district = player.city.at(i);
                if (district.ability && district.ability->canUse(ctx_)) {
                    options.push_back("Use " + district.name);
                    entries.push_back(Entry::District);
                    cityIndexes.push_back(i);
                }
            }
            if (ctx_.character.incomeColor() && !ctx_.incomeTaken) {
                options.push_back("Collect income");
                entries.push_back(Entry::Income);
            }
            if (hasAbilityDistrict(player)) {
                options.push_back("Card help");
                entries.push_back(Entry::Help);
            }
            options.push_back("End turn");
            entries.push_back(Entry::End);

            const std::size_t chosen = static_cast<std::size_t>(ui.choose("Choose:", options));
            const Entry entry = entries[chosen];
            if (entry == Entry::End) {
                return;
            }
            if (entry == Entry::Power) {
                ctx_.character.usePower(ctx_);
                continue;
            }
            if (entry == Entry::District) {
                // This entry is the (chosen - firstDistrict)-th district entry.
                const std::size_t firstDistrict = static_cast<std::size_t>(
                    std::find(entries.begin(), entries.end(), Entry::District) - entries.begin());
                player.city.at(cityIndexes[chosen - firstDistrict]).ability->use(ctx_);
                continue;
            }
            if (entry == Entry::Help) {
                explainDistricts();
                continue;
            }
            if (entry == Entry::Income) {
                ctx_.character.collectIncome(ctx_);
                continue;
            }

            std::vector<std::string> cards;
            for (int index : buildable) {
                const Card& card = player.hand.at(index);
                cards.push_back(card.name + " (" + std::to_string(player.costOf(card)) + ")");
            }
            cards.push_back("Back");
            const int pick = ui.choose("Build which district?", cards);
            if (static_cast<std::size_t>(pick) >= buildable.size()) {
                continue;
            }
            const int handIndex = buildable[static_cast<std::size_t>(pick)];
            const Card toBuild = player.hand.at(handIndex);
            const std::string built = toBuild.name;
            std::vector<int> payWith;
            if (toBuild.ability && toBuild.ability->canBePaidWithCards()) {
                payWith = askPayment(handIndex);
            }
            for (Card& paid : player.build(handIndex, payWith)) {
                ctx_.state.districtDeck.putBottom(std::move(paid));
            }
            --ctx_.buildsLeft;
            ui.announce(player.name + " builds " + built + ".");
            if (player.city.isComplete(ctx_.state.districtsToFinish) && !ctx_.state.firstToComplete) {
                ctx_.state.firstToComplete = player.seat;
                ui.announce(player.name + " has completed their city!");
            }
        }
    }

    // Thieves' Den: how many cards to pay with, then which ones. Returns hand indexes in the order picked.
    std::vector<int> Turn::askPayment(int handIndex) {
        const Player& player = ctx_.player;
        IUserInterface& ui = ctx_.ui;
        const Card& den = player.hand.at(handIndex);

        // The other cards of the hand (the Den itself is not one), in hand order.
        std::vector<int> payable;
        for (int i = 0; i < player.hand.size(); ++i) {
            if (player.hand.at(i).name != den.name) payable.push_back(i);
        }
        const int count = static_cast<int>(payable.size());
        const int price = player.costOf(den);
        const int fewest = std::max(0, price - player.gold);
        const int most = std::min(price, count);

        int cards = fewest;
        if (fewest < most) {
            std::vector<std::string> options;
            for (int n = fewest; n <= most; ++n) {
                if (n == 0)      options.push_back("Pay with gold only");
                else if (n == 1) options.push_back("Pay with 1 card");
                else             options.push_back("Pay with " + std::to_string(n) + " cards");
            }
            cards = fewest + ui.choose("How do you pay for " + den.name + "?", options);
        }

        if (cards == 0) return {};
        if (cards == count) return payable;

        std::vector<int> picks;
        std::vector<int> remaining = payable;
        for (int i = 0; i < cards; ++i) {
            std::vector<std::string> options;
            for (int index : remaining) options.push_back(describe(player.hand.at(index)));
            const int pick = ui.choose("Pay with which card?", options);
            picks.push_back(remaining[static_cast<std::size_t>(pick)]);
            remaining.erase(remaining.begin() + pick);
        }
        return picks;
    }

    // Card help: one line per district with an ability, the city first, then the hand.
    void Turn::explainDistricts() {
        const Player& player = ctx_.player;
        for (int i = 0; i < player.city.size(); ++i) {
            const Card& card = player.city.at(i);
            if (card.ability) ctx_.ui.announce(card.name + ": " + card.ability->description());
        }
        for (int i = 0; i < player.hand.size(); ++i) {
            const Card& card = player.hand.at(i);
            if (card.ability) ctx_.ui.announce(card.name + ": " + card.ability->description());
        }
    }

    void Turn::endOfTurn() {
        ctx_.character.atEndOfTurn(ctx_);
    }
}

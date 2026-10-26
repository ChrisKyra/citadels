#pragma once

#include <memory>
#include <string>

namespace citadels {

    struct Card;
    class Player;
    class GameState;
    struct TurnContext;

    // A special ability of a district. Abilities never change, so cards share them (AbilityPtr).
    // Only id() is required; every hook has a harmless default, and each district overrides
    // just the ones it needs.
    class DistrictAbility {
    public:
        virtual ~DistrictAbility() = default;

        // The id used in the CSV.
        virtual std::string id() const = 0;
        // One line of rules text, shown by "Card help".
        virtual std::string description() const = 0;

        // Extra end-of-game points for `owner`, who holds this district in the city.
        virtual int scoreBonus(const Player& /*owner*/, const GameState& /*state*/) const { return 0; }
        // Counts as any colour for the all-colours bonus.
        virtual bool isScoringWildcard() const { return false; }
        // Counts as a district of the colour of the character collecting income.
        virtual bool isIncomeWildcard() const { return false; }
        // False: the Warlord may not destroy it.
        virtual bool canBeDestroyed() const { return true; }
        // Gold less to pay for `target` (held in the city).
        virtual int buildDiscount(const Card& /*target*/) const { return 0; }
        // Lets the owner build a district he already has (held in the city).
        virtual bool allowsDuplicates() const { return false; }
        // The district itself can be paid with cards (the card being built).
        virtual bool canBePaidWithCards() const { return false; }
        // "Draw 2 cards" keeps both cards (held in the city).
        virtual bool keepsBothDrawnCards() const { return false; }
        // A once-per-turn action is available now; ctx.player is the owner.
        virtual bool canUse(const TurnContext& /*ctx*/) const { return false; }
        virtual void use(TurnContext& /*ctx*/) const {}
    };

    using AbilityPtr = std::shared_ptr<const DistrictAbility>;

    class DragonGate final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int scoreBonus(const Player& owner, const GameState& state) const override;
    };

    class Factory final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int buildDiscount(const Card& target) const override;
    };

    class HauntedQuarter final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool isScoringWildcard() const override;
    };

    class ImperialTreasury final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int scoreBonus(const Player& owner, const GameState& state) const override;
    };

    class Keep final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool canBeDestroyed() const override;
    };

    class Laboratory final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool canUse(const TurnContext& ctx) const override;
        void use(TurnContext& ctx) const override;
    };

    class Library final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool keepsBothDrawnCards() const override;
    };

    class MapRoom final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int scoreBonus(const Player& owner, const GameState& state) const override;
    };

    class Quarry final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool allowsDuplicates() const override;
    };

    class SchoolOfMagic final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool isIncomeWildcard() const override;
    };

    class Smithy final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool canUse(const TurnContext& ctx) const override;
        void use(TurnContext& ctx) const override;
    };

    class Statue final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int scoreBonus(const Player& owner, const GameState& state) const override;
    };

    class ThievesDen final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        bool canBePaidWithCards() const override;
    };

    class WishingWell final : public DistrictAbility {
    public:
        std::string id() const override;
        std::string description() const override;
        int scoreBonus(const Player& owner, const GameState& state) const override;
    };
}

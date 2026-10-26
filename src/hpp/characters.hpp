#pragma once
#include "character_id.hpp"
#include "color.hpp"
#include <optional>
#include <memory>
#include <string>

namespace citadels {

    struct TurnContext;

    class Character {
    public:
        virtual ~Character() = default;

        virtual CharacterId id() const = 0;
        virtual std::string name() const = 0;
        virtual int rank() const = 0;

        // Colour of the districts that pay income to this character, if any.
        virtual std::optional<Color> incomeColor() const { return std::nullopt; }
        // Districts that may be built per turn.
        virtual int buildLimit() const { return 1; }
        // True: the Warlord cannot destroy districts in this character's city.
        virtual bool protectsCity() const { return false; }
        // True: the character has a "Use power" menu entry.
        virtual bool hasPower() const { return false; }

        // Turn hooks; the defaults do nothing.
        virtual void onReveal(TurnContext& /*ctx*/) const {}
        virtual void afterAction(TurnContext& /*ctx*/) const {}
        virtual void usePower(TurnContext& /*ctx*/) const {}
        virtual void atEndOfTurn(TurnContext& /*ctx*/) const {}

        // Template method: pays gold for districts of incomeColor(). Not overridden.
        virtual void collectIncome(TurnContext& ctx) const;
    };

    using CharacterPtr = std::unique_ptr<Character>;

    class Assassin final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        bool hasPower() const override;
        void usePower(TurnContext&) const override;
    };

    class Thief final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        bool hasPower() const override;
        void usePower(TurnContext&) const override;
    };

    class Magician final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        bool hasPower() const override;
        void usePower(TurnContext&) const override;
    };

    class King final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        std::optional<Color> incomeColor() const override;
        void onReveal(TurnContext&) const override;
    };

    class Bishop final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        std::optional<Color> incomeColor() const override;
        bool protectsCity() const override;
    };

    class Merchant final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        std::optional<Color> incomeColor() const override;
        void afterAction(TurnContext&) const override;
    };

    class Architect final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        int buildLimit() const override;
        void afterAction(TurnContext&) const override;
    };

    class Warlord final : public Character {
    public:
        CharacterId id() const override;
        std::string name() const override;
        int rank() const override;
        std::optional<Color> incomeColor() const override;
        void atEndOfTurn(TurnContext&) const override;
    };

}

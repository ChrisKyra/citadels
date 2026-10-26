#pragma once

#include <deque>
#include <string>
#include <vector>
#include "user_interface.hpp"
#include "player.hpp"

namespace citadels {

    // Test double: replays scripted answers and records everything it is shown.
    class ScriptedUI final : public IUserInterface {
    public:
        explicit ScriptedUI(std::vector<int> choices = {}, std::vector<bool> confirms = {});

        void announce(const std::string& msg) override;
        void handOff(const Player& player) override;
        int choose(const std::string& prompt, const std::vector<std::string>& options) override;
        bool confirm(const std::string& prompt) override;
        void showTable(const GameState& state) override;
        void showPrivate(const Player& player) override;

        const std::vector<std::string>& announcements() const;
        const std::vector<std::string>& handOffs() const;   // player names
        const std::vector<std::string>& prompts() const;    // choose and confirm, in call order
        const std::vector<std::string>& lastOptions() const;

        int showTableCount() const;
        const std::vector<std::string>& privateViews() const;   // player names

        int remainingChoices() const;
        int remainingConfirms() const;

    private:
        std::deque<int> choices_;
        std::deque<bool> confirms_;
        std::vector<std::string> announcements_;
        std::vector<std::string> handOffs_;
        std::vector<std::string> prompts_;
        std::vector<std::string> lastOptions_;
        std::vector<std::string> privateViews_;
        int showTableCount_ = 0;
    };
}

#include "scripted_ui.hpp"
#include <stdexcept>
#include <utility>

namespace citadels {

    ScriptedUI::ScriptedUI(std::vector<int> choices, std::vector<bool> confirms)
        : choices_(choices.begin(), choices.end()),
          confirms_(confirms.begin(), confirms.end()) {}

    void ScriptedUI::announce(const std::string& msg) {
        announcements_.push_back(msg);
    }

    void ScriptedUI::handOff(const Player& player) {
        handOffs_.push_back(player.name);
    }

    int ScriptedUI::choose(const std::string& prompt, const std::vector<std::string>& options) {
        prompts_.push_back(prompt);
        lastOptions_ = options;
        if (choices_.empty()) {
            throw std::runtime_error("ScriptedUI: no scripted choice left for prompt: " + prompt);
        }
        const int index = choices_.front();
        if (index < 0 || static_cast<std::size_t>(index) >= options.size()) {
            throw std::out_of_range("ScriptedUI: scripted choice out of range for prompt: " + prompt);
        }
        choices_.pop_front();
        return index;
    }

    bool ScriptedUI::confirm(const std::string& prompt) {
        prompts_.push_back(prompt);
        if (confirms_.empty()) {
            throw std::runtime_error("ScriptedUI: no scripted confirm left for prompt: " + prompt);
        }
        const bool answer = confirms_.front();
        confirms_.pop_front();
        return answer;
    }

    void ScriptedUI::showTable(const GameState&) {
        ++showTableCount_;
    }

    void ScriptedUI::showPrivate(const Player& player) {
        privateViews_.push_back(player.name);
    }

    int ScriptedUI::showTableCount() const { return showTableCount_; }
    const std::vector<std::string>& ScriptedUI::privateViews() const { return privateViews_; }

    const std::vector<std::string>& ScriptedUI::announcements() const { return announcements_; }
    const std::vector<std::string>& ScriptedUI::handOffs() const { return handOffs_; }
    const std::vector<std::string>& ScriptedUI::prompts() const { return prompts_; }
    const std::vector<std::string>& ScriptedUI::lastOptions() const { return lastOptions_; }

    int ScriptedUI::remainingChoices() const { return static_cast<int>(choices_.size()); }
    int ScriptedUI::remainingConfirms() const { return static_cast<int>(confirms_.size()); }
}
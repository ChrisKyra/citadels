#include "protocol_ui.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include "card.hpp"
#include "color.hpp"
#include "district_ability.hpp"
#include "game_state.hpp"

namespace citadels {

    namespace {
        // The text in quotes, escaped for JSON: " \ and the control characters below 0x20.
        // Bytes from 0x20 up (UTF-8 included) are copied unchanged.
        std::string jsonString(const std::string& text) {
            std::string result = "\"";
            for (char c : text) {
                switch (c) {
                    case '"':  result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: {
                        const unsigned char byte = static_cast<unsigned char>(c);
                        if (byte < 0x20) {
                            char escape[8];
                            std::snprintf(escape, sizeof escape, "\\u%04x", static_cast<unsigned>(byte));
                            result += escape;
                        } else {
                            result += c;
                        }
                    }
                }
            }
            result += '"';
            return result;
        }

        const char* colorWord(Color color) {
            switch (color) {
                case Color::Noble:     return "noble";
                case Color::Trade:     return "trade";
                case Color::Religious: return "religious";
                case Color::Military:  return "military";
                case Color::Special:   return "special";
            }
            throw std::logic_error("ProtocolUI: unknown colour");
        }

        std::string cardJson(const Card& card) {
            std::string json = "{\"name\":" + jsonString(card.name);
            json += ",\"cost\":" + std::to_string(card.cost);
            json += ",\"color\":\"" + std::string(colorWord(card.color)) + "\"";
            json += ",\"ability\":";
            json += card.ability ? jsonString(card.ability->description()) : std::string("null");
            json += '}';
            return json;
        }

        // Works for Hand and City: both have size() and at(i).
        template <typename Cards>
        std::string cardListJson(const Cards& cards) {
            std::string json = "[";
            for (int i = 0; i < cards.size(); ++i) {
                if (i > 0) json += ',';
                json += cardJson(cards.at(i));
            }
            json += ']';
            return json;
        }

        // Whole-line integer only: rejects "abc", "2x", "1.5" and values that overflow int.
        bool parseInt(const std::string& text, int& value) {
            try {
                std::size_t pos = 0;
                value = std::stoi(text, &pos);
                return pos == text.size();
            } catch (const std::invalid_argument&) {
                return false;
            } catch (const std::out_of_range&) {
                return false;
            }
        }
    }

    ProtocolUI::ProtocolUI(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

    void ProtocolUI::send(const std::string& line) {
        out_ << line << '\n' << std::flush;
    }

    void ProtocolUI::sendError(const std::string& text) {
        send("{\"type\":\"error\",\"text\":" + jsonString(text) + "}");
    }

    std::string ProtocolUI::readLine(const char* where) {
        std::string line;
        if (!std::getline(in_, line)) {
            throw std::runtime_error(std::string("ProtocolUI::") + where + ": end of input");
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return line;
    }

    void ProtocolUI::announce(const std::string& msg) {
        send("{\"type\":\"announce\",\"text\":" + jsonString(msg) + "}");
    }

    void ProtocolUI::handOff(const Player& player) {
        send("{\"type\":\"handOff\",\"name\":" + jsonString(player.name) +
             ",\"seat\":" + std::to_string(player.seat) + "}");
        readLine("handOff");   // whatever the other program answers means "ready"
    }

    int ProtocolUI::choose(const std::string& prompt, const std::vector<std::string>& options) {
        if (options.empty()) {
            throw std::invalid_argument("ProtocolUI::choose: no options");
        }
        std::string question = "{\"type\":\"choose\",\"prompt\":" + jsonString(prompt) + ",\"options\":[";
        for (std::size_t i = 0; i < options.size(); ++i) {
            if (i > 0) question += ',';
            question += jsonString(options[i]);
        }
        question += "]}";

        const int count = static_cast<int>(options.size());
        while (true) {
            send(question);
            int n = 0;
            if (parseInt(readLine("choose"), n) && n >= 0 && n < count) {
                return n;
            }
            sendError("Please answer with a number from 0 to " + std::to_string(count - 1) + ".");
        }
    }

    bool ProtocolUI::confirm(const std::string& prompt) {
        const std::string question = "{\"type\":\"confirm\",\"prompt\":" + jsonString(prompt) + "}";
        while (true) {
            send(question);
            std::string line = readLine("confirm");
            std::transform(line.begin(), line.end(), line.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (line == "y" || line == "yes") return true;
            if (line == "n" || line == "no") return false;
            sendError("Please answer y or n.");
        }
    }

    void ProtocolUI::showTable(const GameState& state) {
        std::string json = "{\"type\":\"table\",\"round\":" + std::to_string(state.roundNumber) +
                           ",\"crown\":" + std::to_string(state.crownHolder) + ",\"players\":[";
        for (std::size_t i = 0; i < state.players.size(); ++i) {
            const Player& player = state.players[i];
            if (i > 0) json += ',';
            json += "{\"seat\":" + std::to_string(player.seat);
            json += ",\"name\":" + jsonString(player.name);
            json += ",\"gold\":" + std::to_string(player.gold);
            json += ",\"handSize\":" + std::to_string(player.hand.size());
            json += ",\"city\":" + cardListJson(player.city) + "}";
        }
        json += "]}";
        send(json);
    }

    void ProtocolUI::showPrivate(const Player& player) {
        send("{\"type\":\"private\",\"name\":" + jsonString(player.name) +
             ",\"seat\":" + std::to_string(player.seat) +
             ",\"gold\":" + std::to_string(player.gold) +
             ",\"hand\":" + cardListJson(player.hand) + "}");
    }
}

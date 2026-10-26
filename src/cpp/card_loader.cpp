#include "card_loader.hpp"
#include "ability_factory.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace citadels {

    // Helpers that only this file uses (unnamed namespace = private to card_loader.cpp).
    namespace {

        // Turns a colour word from the CSV into a Color.
        // Throws std::invalid_argument for an unknown word.
        Color parseColor(const std::string& word) {
            if (word == "noble")     return Color::Noble;
            if (word == "trade")     return Color::Trade;
            if (word == "religious") return Color::Religious;
            if (word == "military")  return Color::Military;
            if (word == "special")   return Color::Special;
            throw std::invalid_argument("Unknown district colour: '" + word + "'");
        }

        // Turns a WHOLE string into an int. "3" -> 3, but "abc", "3x" and "" all throw
        // std::invalid_argument (std::stoi alone would accept "3x" and return 3).
        int parseInt(const std::string& text, const std::string& what) {
            std::size_t used = 0;   // how many characters stoi consumed
            int value = 0;
            try {
                value = std::stoi(text, &used);
            } catch (const std::exception&) {   // stoi throws invalid_argument or out_of_range
                throw std::invalid_argument("Bad number for " + what + ": '" + text + "'");
            }
            if (used != text.size()) {
                throw std::invalid_argument("Bad number for " + what + ": '" + text + "'");
            }
            return value;
        }

        // Splits "a,b,c" into {"a", "b", "c"}.
        // Note: getline drops an empty LAST field, so "Manor,noble,3,5," gives 4 fields, not 5.
        std::vector<std::string> splitLine(const std::string& line) {
            std::vector<std::string> fields;
            std::istringstream stream(line);
            std::string field;
            while (std::getline(stream, field, ',')) {
                fields.push_back(field);
            }
            return fields;
        }

    }  // namespace

    std::vector<Card> CardLoader::loadDistricts(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open district file: " + filename);
        }

        std::vector<Card> cards;
        std::string line;

        std::getline(file, line);   // the first line is the header: read it and throw it away

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {   // Windows line ending
                line.pop_back();
            }
            if (line.empty()) {                           // blank line
                continue;
            }

            std::vector<std::string> fields = splitLine(line);
            if (fields.size() < 4) {                      // need name, color, cost, copies
                throw std::invalid_argument("Too few columns in line: '" + line + "'");
            }

            const std::string& name = fields[0];
            Color color = parseColor(fields[1]);
            int cost = parseInt(fields[2], "cost");
            int copies = parseInt(fields[3], "copies");
            if (copies < 0) {
                throw std::invalid_argument("Negative number of copies in line: '" + line + "'");
            }

            // The ability column is optional; an empty or missing one means no ability.
            const AbilityPtr ability = AbilityFactory::create(fields.size() > 4 ? fields[4] : "");

            for (int i = 0; i < copies; ++i) {
                cards.emplace_back(name, cost, color, ability);    // Card(name, cost, color, ability)
            }
        }

        return cards;
    }
}

#pragma once

#include <string>
#include <vector>
#include "card.hpp"

namespace citadels {

    // Reads the district CSV (columns: name,color,cost,copies,ability).
    // Every row becomes `copies` identical cards, in file order (no shuffling).
    // The ability column is ignored for now.
    //
    // Throws std::runtime_error if the file can't be opened,
    // std::invalid_argument for a bad row.
    class CardLoader {
    public:
        static std::vector<Card> loadDistricts(const std::string& filename);
    };

}

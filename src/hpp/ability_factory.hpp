#pragma once

#include <string>
#include "district_ability.hpp"

namespace citadels {

    class AbilityFactory {
    public:
        // "" gives nullptr (no ability). Throws std::invalid_argument for an unknown id;
        // ids are case sensitive.
        static AbilityPtr create(const std::string& id);
    };
}

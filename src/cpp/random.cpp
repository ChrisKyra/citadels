/* Implement the random header */

#include "random.hpp"

namespace citadels {

    int Random::uniform(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(engine_);
    }

    std::mt19937& Random::engine() {
        return engine_;
    }

}
#pragma once
#include <random>
#include <cstdint>

namespace citadels {

    class Random {
    public:
        explicit Random(std::uint32_t seed) : engine_(seed) {}

        int uniform(int lo, int hi);

        std::mt19937& engine();

    private:
        std::mt19937 engine_;
    };

}
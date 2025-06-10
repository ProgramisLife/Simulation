#pragma once
#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <cstdint>
#include <cmath>

class RandomGenerator {
private:
    uint32_t seed;  // Ziarno generatora

public:
    explicit RandomGenerator(uint32_t initial_seed = 123456789);

    void set_seed(uint32_t new_seed);
    uint32_t next();  // Generuje kolejną liczbę pseudolosową (XORSHIFT32)

    // Losowa liczba z przedziału [min, max]
    int uniform_int(int min, int max);

    // Losowa liczba z rozkładu wykładniczego (dla czasu aktywności μ)
    double exponential(double lambda);

    // Losowa liczba z przedziału [min, max] (dla czasu aktywacji U1)
    double uniform_real(double min, double max);
};

#endif // RANDOM_GENERATOR_H
#pragma once

#include "random_generator.h"
#include <stdexcept>
#include <limits>
#include "constants.h"

RandomGenerator::RandomGenerator(uint32_t initial_seed)
    : seed(initial_seed) {
}

void RandomGenerator::set_seed(uint32_t new_seed) {
    seed = new_seed;
}

uint32_t RandomGenerator::next() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

int RandomGenerator::uniform_int(int min, int max) {
    if (min >= max) return min;
    return min + (next() % (max - min + 1));
}

double RandomGenerator::exponential(double lambda) {
    if (lambda <= 0.0) throw std::invalid_argument("Lambda must be positive");
    double u = next() / static_cast<double>(std::numeric_limits<uint32_t>::max());
    return -log(1.0 - u) / lambda;
}

double RandomGenerator::uniform_real(double min, double max) {
    if (min >= max) return min;
    double scale = next() / static_cast<double>(std::numeric_limits<uint32_t>::max());
    return min + scale * (max - min);
}
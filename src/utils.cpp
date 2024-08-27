#include <fastNoiseLite/fastNoiseLite.h>
#include <vector>
#include "utils.hpp"

using namespace engine;

std::vector<u_char> PerlinNoise::generate_char(int n, float freq, float seed) {
    FastNoiseLite noise{};
    noise.SetSeed(seed);
    noise.SetFrequency(freq);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    std::vector<u_char> noiseOutput(n*n);

    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            float value = noise.GetNoise(static_cast<float>(x),static_cast<float>(y));
            noiseOutput[n*y+x] = static_cast<u_char>(((value+1.)/2.)*255.);
        }
    }

    return noiseOutput;
}

std::vector<float> PerlinNoise::generate(int n, float freq, float seed) {
    FastNoiseLite noise{};
    noise.SetSeed(seed);
    noise.SetFrequency(freq);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    std::vector<float> noiseOutput(n*n);

    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            float value = noise.GetNoise(static_cast<float>(x),static_cast<float>(y));
            noiseOutput[n*y+x] = (value+1.)/2.;
        }
    }

    return noiseOutput;
}
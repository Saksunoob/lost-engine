#include <fastNoiseLite/fastNoiseLite.h>
#include <vector>
#include "utils.hpp"
#include "components.hpp"

using namespace engine;

Vector2::Vector2(IVector2 vec) : x(vec.x), y(vec.y) {};

Vector2 Vector2::operator*(Transform& transform) const {
    glm::vec4 transformed = glm::vec4(x, y, 0.0, 1.0) * transform.getTransformationMatrix();
    return {transformed.x, transformed.y};
}

Polygon Polygon::transformed(Transform& transform) {
    Polygon transformed = *this;
    for (Vector2& point : transformed.points) {
        point = point * transform;
    }
    return transformed;
}

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
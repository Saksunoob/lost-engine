#pragma once

namespace engine {
    struct Vector2 {
        float x,y;

        Vector2(float x, float y) : x(x), y(y) {};
    };

    struct IVector2 {
        int x,y;

        IVector2() : x(0), y(0) {};
        IVector2(int x, int y): x(x), y(y) {};
    };

    struct Color {
        float r, g, b, a;

        Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};
        Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0) {};
        Color(float brightness) : r(brightness), g(brightness), b(brightness), a(1.0) {};
    };

    struct PerlinNoise {
        static std::vector<u_char> generate_char(int n, float freq, float seed);
        static std::vector<float> generate(int n, float freq, float seed);
    };
}
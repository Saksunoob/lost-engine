#pragma once

namespace engine {
    struct Vector2 {
        double x,y;

        Vector2(double x, double y) : x(x), y(y) {};
    };

    struct Color {
        float r, g, b, a;

        Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};
        Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0) {};
        Color(float brightness) : r(brightness), g(brightness), b(brightness), a(1.0) {};
    };
}
#pragma once

#include "logger.hpp"

namespace engine {
    struct IVector2;
    struct Transform;

    struct Vector2 {
        float x,y;

        Vector2() : x(0), y(0) {};
        Vector2(IVector2 vec);
        Vector2(float x, float y) : x(x), y(y) {};

        Vector2 normal() {
            return Vector2(-y, x);
        }

        float magnitude() const {
            return sqrt(x*x+y*y);
        }

        float distance(Vector2 other) const {
            return operator-(other).magnitude();
        }

        float dot(Vector2 other) {
            return x*other.x+y*other.y;
        }

        Vector2 operator+(Vector2 other) const {
            return Vector2(x+other.x,y+other.y);
        }
        Vector2 operator-(Vector2 other) const {
            return Vector2(x-other.x,y-other.y);
        }
        Vector2 operator-() const {
            return Vector2(-x,-y);
        }
        Vector2 operator*(Vector2 other) const {
            return Vector2(x*other.x,y*other.y);
        }
        Vector2 operator/(Vector2 other) const {
            return Vector2(x/other.x,y/other.y);
        }
        Vector2 operator*(float other) const {
            return Vector2(x*other,y*other);
        }
        Vector2 operator*(Transform& transform) const;
        Vector2 operator/(float other) const {
            return Vector2(x/other,y/other);
        }
        Vector2 rotate(float radians) const {
            float cs = cos(radians);
            float sn = sin(radians);
            return Vector2(x*cs-y*sn, x*sn+y*cs);
        }
    };

    struct IVector2 {
        int x,y;

        IVector2() : x(0), y(0) {};
        IVector2(int x, int y): x(x), y(y) {};

        bool operator==(IVector2 other) const {
            return x==other.x && y==other.y;
        }
        IVector2 operator+(IVector2 other) const {
            return IVector2(x+other.x,y+other.y);
        }
        IVector2 operator-(IVector2 other) const {
            return IVector2(x-other.x,y-other.y);
        }
        IVector2 operator-() const {
            return IVector2(-x,-y);
        }
        IVector2 operator*(IVector2 other) const {
            return IVector2(x*other.x,y*other.y);
        }
        IVector2 operator/(IVector2 other) const {
            return IVector2(x/other.x,y/other.y);
        }
        IVector2 operator*(int mul) const {
            return IVector2(x*mul,y*mul);
        }
        IVector2 operator/(int div) const {
            return IVector2(x/div,y/div);
        }
    };

    struct Color {
        float r, g, b, a;

        Color() : Color(0,0,0,0) {};
        Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};
        Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0) {};
        Color(float brightness) : r(brightness), g(brightness), b(brightness), a(1.0) {};
    };

    struct Polygon {
        std::vector<Vector2> points;

        Polygon transformed(Transform& transform);
    };

    struct AABB {
        Vector2 minp, maxp;

        AABB(Vector2 p1, Vector2 p2) {
            minp = Vector2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
            maxp = Vector2(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
        }

        bool collidesWithPoint(Vector2 point) {
            return point.x >= minp.x && point.y >= minp.y && point.x <= maxp.x && point.y <= maxp.y;
        }

        Polygon getPolygon() {
            return {{minp, {minp.x, maxp.y}, maxp, {maxp.x, minp.y}}};
        }
    };

    struct PerlinNoise {
        static std::vector<u_char> generate_char(int n, float freq, float seed);
        static std::vector<float> generate(int n, float freq, float seed);
    };

    template <typename T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    template<typename Tuple, std::size_t... Is>
    inline void hash_tuple_combine_impl(std::size_t& seed, const Tuple& tuple, std::index_sequence<Is...>)
    {
        (..., hash_combine(seed, std::get<Is>(tuple)));
    }

    template <typename... T>
    struct tuple_hash {
        std::size_t operator () (const std::tuple<T...>& tuple) const {
            std::size_t seed = 0;
            hash_tuple_combine_impl(seed, tuple, std::make_index_sequence<sizeof...(T)>{});
            return seed;
        }
    };
}
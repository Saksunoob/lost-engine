#include <fastNoiseLite/fastNoiseLite.h>
#include <vector>
#include "utils.hpp"
#include "components.hpp"

using namespace engine;

Vector2::Vector2(IVector2 vec) : x(vec.x), y(vec.y) {};

Vector2 Vector2::operator*(const Transform& transform) const {
    glm::vec4 transformed = glm::vec4(x, y, 0.0, 1.0) * transform.getTransformationMatrix();
    return {transformed.x, transformed.y};
}

Polygon Polygon::transformed(const Transform& transform) const {
    Polygon transformed = *this;
    for (Vector2& point : transformed.points) {
        point = point * transform;
    }
    return transformed;
}
std::vector<Vector2> Polygon::getAxes() const {
    std::vector<Vector2> axes(points.size());
    for (int i = 0; i < points.size(); i++) {
        Vector2 edge = points[i] - points[(i+1)%points.size()];
        Vector2 normal = edge.normal();
        axes[i] = (normal / normal.magnitude());
    }
    return axes;
}

std::array<float, 2> Polygon::project(Vector2 axis) const {
    float dot = points.at(0).dot(axis);
    float min = dot;
    float max = dot;
    for (int i = 1; i < points.size(); i++) {
        float dot = points[i].dot(axis);
        min = std::min(min, dot);
        max = std::max(max, dot);
    }
    return {min, max};
}

bool Polygon::overlaps(const Polygon& other) const {
    std::vector<Vector2> axes = getAxes();
    std::vector<Vector2> axes2 = other.getAxes();
    axes.insert(axes.end(), axes2.begin(), axes2.end());

    for (Vector2 axis : axes) {
        std::array<float, 2> proj1 = project(axis);
        std::array<float, 2> proj2 = other.project(axis);

        if (proj1[1] < proj2[0] || proj2[1] < proj1[0]) {
            return false;
        }
    }
    return true;
}

Polygon Polygon::getHull() const {
    std::vector<Vector2> c_points = points;

    std::sort(c_points.begin(), c_points.end(), [](const Vector2& a, const Vector2& b) {
        return a.x == b.x ? a.y < b.y : a.x < b.x;
    });

    std::vector<Vector2> hull;

    // Lower hull
    for (const auto& p : c_points) {
        while (hull.size() >= 2 && (hull[hull.size() - 1] - hull[hull.size() - 2]).cross(p - hull[hull.size() - 1]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(p);
    }

    // Upper hull
    std::size_t lowerHullSize = hull.size();
    for (auto it = c_points.rbegin(); it != c_points.rend(); ++it) {
        while (hull.size() > lowerHullSize && (hull[hull.size() - 1] - hull[hull.size() - 2]).cross(*it - hull[hull.size() - 1]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(*it);
    }

    hull.pop_back(); // The last point is the same as the first point.
    return Polygon(hull);
}

OBB Polygon::getOBB() const {
    Polygon hull = getHull();

    float minArea = std::numeric_limits<float>::max();
    Vector2 bestCenter, bestSize;
    float bestAngle = 0;

    for (std::size_t i = 0; i < hull.points.size(); ++i) {
        // Get edge vector
        Vector2 edge = hull.points[(i + 1) % hull.points.size()] - hull.points[i];

        // Calculate edge angle
        float edgeAngle = std::atan2(edge.y, edge.x);
        float cosAngle = std::cos(edgeAngle);
        float sinAngle = std::sin(edgeAngle);

        // Rotate all points by -edgeAngle to align with the x-axis
        float minX = std::numeric_limits<float>::max(), minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest(), maxY = std::numeric_limits<float>::lowest();

        for (const auto& p : hull.points) {
            float rotatedX = cosAngle * p.x + sinAngle * p.y;
            float rotatedY = -sinAngle * p.x + cosAngle * p.y;

            minX = std::min(minX, rotatedX);
            minY = std::min(minY, rotatedY);
            maxX = std::max(maxX, rotatedX);
            maxY = std::max(maxY, rotatedY);
        }

        float width = maxX - minX;
        float height = maxY - minY;
        float area = width * height;

        if (area < minArea) {
            minArea = area;
            bestCenter = { (minX + maxX) / 2.0f, (minY + maxY) / 2.0f };
            bestSize = { width, height };
            bestAngle = edgeAngle;
        }
    }

    return OBB(bestCenter, bestSize, bestAngle);
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
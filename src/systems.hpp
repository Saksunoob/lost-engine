#pragma once

#include <chrono>

#include "engine.hpp"
#include "shader.hpp"

namespace engine {
    void renderColorMeshes(Scene& scene);
    void renderUVMeshes(Scene& scene);
    void renderTileMaps(Scene& scene);
    void updateUITransforms(Scene& scene);
    void renderColorUI(Scene& scene);
    void timeSystem(Scene& scene);
    void pollSDLEvents(Scene& scene);
}
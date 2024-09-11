#pragma once

#include <chrono>
#include <functional>

#include "engine.hpp"
#include "shader.hpp"
#include "stage.hpp"

namespace engine {
    void renderColorMeshes(Scene& scene);
    void renderUVMeshes(Scene& scene);
    void renderTileMaps(Scene& scene);
    void renderIndexedTextures(Scene& scene);
    void updateTransforms(Scene& scene);
    void updateUITransforms(Scene& scene);
    void renderSlicedTextures(Scene& scene);
    void timeSystem(Scene& scene);
    void pollSDLEvents(Scene& scene);

    static const std::vector<Stage::System> DEFAULT_INIT_SYSTEMS = {
        pollSDLEvents,
        timeSystem
    };

    static const std::vector<Stage::System> DEFAULT_UPDATE_SYSTEMS = {
        updateTransforms,
        updateUITransforms
    };

    static const std::vector<Stage::System> DEFAULT_RENDER_SYSTEMS = {
        renderColorMeshes,
        renderUVMeshes,
        renderTileMaps,
        renderIndexedTextures,
        renderSlicedTextures
    };
}
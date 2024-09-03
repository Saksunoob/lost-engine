#pragma once

#include "bundle.hpp"
#include "components.hpp"

namespace engine {
    class Bundles {
        public:
        static Bundle& quadMeshBundle() {
            static Vertices vertices({
                {0.5, 0.5},
                {-0.5, -0.5},
                {0.5, -0.5},
                {-0.5, 0.5}
            });
            static UVs uvs({
                {1, 1},
                {0, 0},
                {1, 0},
                {0, 1}
            });
            static Indices indices({
                0, 1, 2, 0, 3, 1
            });

            static Bundle quad_mesh_bundle(vertices, indices, uvs);
            return quad_mesh_bundle;
        }
    };
}
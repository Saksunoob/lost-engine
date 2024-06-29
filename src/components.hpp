#include <vector>
#include <memory>
#include <any>

#include "vectors.hpp"

namespace engine {
    template <typename C>
    class Components {
        std::vector<std::unique_ptr<std::any>>* components;

        public:

            Components(std::vector<std::unique_ptr<std::any>>* components) : components(components) {};

            C& operator[](unsigned index) {
                std::any* component = components->operator[](index).get();
                return *std::any_cast<C>(component);
            }

            unsigned size() {
                return components->size();
            }
    };

    struct Transform {
        Vector2 position, scale;
        double rotation;
    };

    struct GlobalTransform : Transform {};

    struct Mesh {
        std::vector<Vector2> verticies;
        std::vector<unsigned> indicies;
    };

    struct UVMesh : Mesh {
        std::vector<Vector2> UVs;
    };

    struct ColorMesh : Mesh {
        std::vector<Color> colors;
    };

    struct Camera {
        bool main;
    };
}
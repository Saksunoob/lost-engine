#include <vector>
#include <memory>
#include <any>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

#include "logger.hpp"
#include "utils.hpp"
#include "vulkan/device.hpp"

namespace engine {
    template <typename C>
    class Components {
        std::vector<std::unique_ptr<std::any>>* components;

        public:

            Components(std::vector<std::unique_ptr<std::any>>* components) : components(components) {};

            C* operator[](unsigned index) {
                if (components == nullptr) {
                    return nullptr;
                }
                std::any* component = components->operator[](index).get();
                return std::any_cast<C>(component);
            }

            unsigned size() {
                if (components == nullptr) {
                    return 0;
                }
                return components->size();
            }
    };

    struct Transform {
        Vector2 position, scale;
        double rotation;

        Transform(Vector2 position, Vector2 scale, double rotation) : position(position), scale(scale), rotation(rotation) {};

        glm::mat4 getTransformationMatrix();
    };

    struct GlobalTransform : Transform {
        GlobalTransform(Vector2 position, Vector2 scale, double rotation): Transform(position, scale, rotation) {};
    };

    struct Mesh {
        std::vector<Vector2> vertices;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        unsigned vertexCount;

        Mesh(std::vector<Vector2> vertices) : vertices(vertices) {};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct UVMesh : Mesh {
        std::vector<Vector2> UVs;

        UVMesh(std::vector<Vector2> verticies, std::vector<Vector2> UVs, std::vector<unsigned> indicies) : Mesh(verticies), UVs(UVs) {};

        //void bind();
    };

    struct ColorMesh : Mesh {
        std::vector<Color> colors;

        ColorMesh(std::vector<Vector2> verticies, std::vector<Color> colors, std::vector<unsigned> indicies) : Mesh(verticies), colors(colors) {};
    };

    struct Camera {
        bool main;

        Camera(bool main) : main(main) {};

        static glm::mat4 getProjectionMatrix(Transform& transform, IVector2 window_size);
    };

    struct Texture {
        unsigned int textureID;
		IVector2 size;

        Texture(Color color, IVector2 size);
        Texture(const char* file_path);

        void use(unsigned);
    };
}
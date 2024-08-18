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

        glm::mat4 getTransformationMatrix() const;
    };

    struct GlobalTransform : Transform {
        GlobalTransform(Vector2 position, Vector2 scale, double rotation): Transform(position, scale, rotation) {};
    };

    struct Mesh {
        std::vector<Vector2> vertices;
        std::vector<unsigned> indices;

        VkBuffer vertexBuffer = nullptr;
        VkBuffer indexBuffer = nullptr;

        Mesh(std::vector<Vector2> vertices, std::vector<unsigned> indices) : vertices(vertices), indices(indices) {};
        ~Mesh() {
            if (_device != nullptr) { // Check if any buffers have been created
                vkDestroyBuffer(_device->device(), vertexBuffer, nullptr);
                vkDestroyBuffer(_device->device(), indexBuffer, nullptr);

                vkFreeMemory(_device->device(), vertexBufferMemory, nullptr);
                vkFreeMemory(_device->device(), indexBufferMemory, nullptr);
            }
        }

        void createBuffers(Device& device);

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        private:
            VkDeviceMemory vertexBufferMemory = nullptr;
            VkDeviceMemory indexBufferMemory = nullptr;
            Device* _device = nullptr;
    };

    struct UVMesh : Mesh {
        std::vector<Vector2> UVs;

        UVMesh(std::vector<Vector2> verticies, std::vector<Vector2> UVs, std::vector<unsigned> indicies) : Mesh(verticies, indicies), UVs(UVs) {};

        //void bind();
    };

    struct ColorMesh : Mesh {
        std::vector<Color> colors;

        ColorMesh(std::vector<Vector2> verticies, std::vector<Color> colors, std::vector<unsigned> indicies) : Mesh(verticies, indicies), colors(colors) {};
    };

    struct Camera {
        bool main;

        Camera(bool main) : main(main) {};

        static glm::mat4 getProjectionMatrix(const Transform& transform, IVector2 window_size);
    };

    struct Texture {
        unsigned int textureID;
		IVector2 size;

        Texture(Color color, IVector2 size);
        Texture(const char* file_path);

        void use(unsigned);
    };
}
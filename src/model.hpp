#pragma once

#include "vulkan/device.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace engine {
    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    class Model {
        Device& device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        unsigned vertexCount;

        void createVertexBuffers(const std::vector<Vertex>& verticies);

        public:
            Model(Device& device, const std::vector<Vertex>& verticies);
            ~Model();

            Model(const Model&) = delete;
            Model &operator=(const Model&) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);
    };
}
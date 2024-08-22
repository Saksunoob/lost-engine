#pragma once

#include <vulkan/vulkan.hpp>
#include "vulkan/pipeline.hpp"
#include "vulkan/device.hpp"
#include "engine.hpp"


namespace engine {
    class Shader;

    class DescriptorPool {
        const unsigned STARTING_POOL_SIZE = 16;

        std::vector<std::vector<VkDescriptorPool>> pools;

        struct Reserve {
            unsigned written;
            std::vector<VkDescriptorSet> sets;

            bool full() {
                return written >= sets.size();
            }

            VkDescriptorSet next() {
                return sets[written++];
            }
        };

        std::vector<std::unordered_map<VkDescriptorSetLayout, Reserve>> reserves;

        unsigned currently_allocated = 0;
        unsigned pool_size;
        unsigned last_image;

        inline unsigned getPoolIndex();
        void createDescriptorPool(std::vector<VkDescriptorPool>& pool);

        public:

        DescriptorPool();
        VkDescriptorSet writeDescriptorSet(VkDescriptorSetLayout set_layout, VkWriteDescriptorSet write);
    };

    enum ShaderVarType {
        VAR_FLOAT = VK_FORMAT_R32_SFLOAT,
        VAR_INT = VK_FORMAT_R32_SINT,
        VAR_UINT = VK_FORMAT_R32_UINT,
        VAR_VEC2 = VK_FORMAT_R32G32_SFLOAT,
        VAR_IVEC2 = VK_FORMAT_R32G32_SINT,
        VAR_UVEC2 = VK_FORMAT_R32G32_UINT,
        VAR_VEC3 = VK_FORMAT_R32G32B32_SFLOAT,
        VAR_IVEC3 = VK_FORMAT_R32G32B32_SINT,
        VAR_UVEC3 = VK_FORMAT_R32G32B32_UINT,
        VAR_VEC4 = VK_FORMAT_R32G32B32A32_SFLOAT,
        VAR_IVEC4 = VK_FORMAT_R32G32B32A32_SINT,
        VAR_UVEC4 = VK_FORMAT_R32G32B32A32_UINT
    };
    struct ShaderVariables {
        std::vector<ShaderVarType> types;

        ShaderVariables(std::vector<ShaderVarType> types) : types(types) {};

        unsigned getTotalSize();
        unsigned getVariableSize(unsigned index);

        std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    template <int UsageFlag>
    class Buffer {
        size_t item_size;
        
        VkDeviceMemory memory = nullptr;
        unsigned bindingSize = 0;

        static unsigned getVarTypeSize(ShaderVarType type);

        public:
            VkBuffer buffer = nullptr;

            Buffer(size_t item_size) : item_size(item_size) {}

            //Buffer(Buffer&) = delete;
            Buffer operator=(Buffer&) = delete;

            ~Buffer() {
                VkDevice device = Engine::getDevice().device();
                if (buffer == nullptr) {
                    Logger::logWarning("destroying null buffer");
                }
                vkDestroyBuffer(device, buffer, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }

            virtual void bind()=0;

            inline void set(const void *data) {
                setVector(data, 1);
            }

            void setVector(const void *data, size_t size) {
                Device& device = Engine::getDevice();
                VkDeviceSize bufferSize = size * item_size;

                if (buffer == nullptr) {
                    device.createBuffer(bufferSize, UsageFlag,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    buffer, memory);
                }

                void* data_loc;
                vkMapMemory(device.device(), memory, 0, bufferSize, 0, &data_loc);
                memcpy(data_loc, data, bufferSize);
                vkUnmapMemory(device.device(), memory);
            }
    };
    class VertexBuffer : public Buffer<VK_BUFFER_USAGE_VERTEX_BUFFER_BIT> {
        using Buffer::Buffer;

        public:
            void bind() override {
                VkDeviceSize offset[] = {0};
                vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, &buffer, offset);
            }
    };
    class IndexBuffer : public Buffer<VK_BUFFER_USAGE_INDEX_BUFFER_BIT> {
        using Buffer::Buffer;

        public:
            void bind() override {
                vkCmdBindIndexBuffer(Engine::getCurrentCommandBuffer(), buffer, 0, VK_INDEX_TYPE_UINT32);
            }
    };

    class UniformBuffer : public Buffer<VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT> {
        using Buffer::Buffer;

        void bind() override {
            Logger::logWarning("Calling bind() on uniform buffer does nothing!");
            return;
        }
    };

    class Shader {
        const char* shaderPath;

        Pipeline* pipeline;
        unsigned pushConstantSize;
        unsigned uniformSize;

        VkDescriptorSetLayout descriptorSetLayout;

        static DescriptorPool* descriptorPool;

        ShaderVariables variables;
        std::vector<std::vector<std::unique_ptr<UniformBuffer>>> uniformBuffers;

        public:
            Shader(const char* shaderPath, ShaderVariables variables, unsigned pushConstantSize, unsigned uniformSize);
            ~Shader();

            void recreate();
            void bind();
            void pushConstant(const void* data, unsigned size);
            void bindUniform(const void* uniform);
    };
}
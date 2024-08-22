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
    class ShaderBuffer {
        size_t item_size;

        static unsigned getVarTypeSize(ShaderVarType type);

        public:
            ShaderBuffer(size_t item_size) : item_size(item_size) {}

            ~ShaderBuffer() {
                VkDevice device = Engine::getDevice().device();
                for (VkBuffer buffer : buffers) {
                    vkDestroyBuffer(device, buffer, nullptr);
                }
                for (VkDeviceMemory memory : memories) {
                    vkFreeMemory(device, memory, nullptr);
                }
            }

            VkBuffer getLast() {
                return buffers[currentBufferIndex-1];
            }

            template <typename T>
            void bind(T) {};
            
            protected:
                unsigned currentBufferIndex = 0;
                std::vector<VkBuffer> buffers;
                std::vector<VkDeviceMemory> memories;
                unsigned bindingSize = 0;

                void set(const void *data, size_t size) {
                    Device& device = Engine::getDevice();

                    if (buffers.size() <= currentBufferIndex) {
                        buffers.resize(currentBufferIndex+1);
                        memories.resize(currentBufferIndex+1);
                    }

                    VkBuffer& buffer = buffers[currentBufferIndex];
                    VkDeviceMemory& memory = memories[currentBufferIndex];

                    if (buffer != nullptr) {
                        vkDestroyBuffer(device.device(), buffer, nullptr);
                        vkFreeMemory(device.device(), memory, nullptr);
                    }

                    VkDeviceSize bufferSize = size;
                    device.createBuffer(bufferSize, UsageFlag,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        buffer, memory);
                    void* data_loc;
                    vkMapMemory(device.device(), memory, 0, bufferSize, 0, &data_loc);
                    memcpy(data_loc, data, size);
                    vkUnmapMemory(device.device(), memory);
                }

            friend Shader;
    };
    class ShaderVertexBuffer : public ShaderBuffer<VK_BUFFER_USAGE_VERTEX_BUFFER_BIT> {
        using ShaderBuffer::ShaderBuffer;

        public:
            template <typename T>
            void bind(const std::vector<T>& vertices) {
                set(vertices.data(), vertices.size() * sizeof(T));
                VkDeviceSize offset[] = {0};
                vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, &buffers[currentBufferIndex], offset);
                currentBufferIndex++;
            }
    };
    class ShaderIndexBuffer : public ShaderBuffer<VK_BUFFER_USAGE_INDEX_BUFFER_BIT> {
        using ShaderBuffer::ShaderBuffer;

        public:
            void bind(const std::vector<unsigned>& indices) {
                set(indices.data(), indices.size() * sizeof(unsigned));
                vkCmdBindIndexBuffer(Engine::getCurrentCommandBuffer(), buffers[currentBufferIndex], 0, VK_INDEX_TYPE_UINT32);
                currentBufferIndex++;
            }
    };

    class ShaderUniformBuffer : public ShaderBuffer<VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT> {
        using ShaderBuffer::ShaderBuffer;

        public:
            template <typename T>
            void bind(const T& uniform) {
                set(&uniform, sizeof(T));
                currentBufferIndex++;
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

        struct PerImageData {
            ShaderVertexBuffer vertexBuffer;
            ShaderIndexBuffer indexBuffer;
            ShaderUniformBuffer uniformBuffer;

            PerImageData(ShaderVariables variables, unsigned uniformSize, VkDescriptorSetLayout layout) : 
            vertexBuffer(variables.getTotalSize()), indexBuffer(sizeof(unsigned)), uniformBuffer(uniformSize) {};
        };

        std::vector<PerImageData> perImageData;

        public:
            Shader(const char* shaderPath, ShaderVariables variables, unsigned pushConstantSize, unsigned uniformSize);
            ~Shader();

            void recreate();
            void bind();
            void pushConstant(const void* data, unsigned size) {
                vkCmdPushConstants(Engine::getCurrentCommandBuffer(), pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
            }

            template <typename T>
            void bindUniform(const T& uniform) {
                ShaderUniformBuffer& buffer = perImageData[Engine::getCurrentSwapChainImage()].uniformBuffer;

                buffer.bind(uniform);
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer.getLast();;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(T);

                VkWriteDescriptorSet writeDescriptorSet = {};
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding = 0;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.pBufferInfo = &bufferInfo;

                VkDescriptorSet descriptorSet = descriptorPool->writeDescriptorSet(descriptorSetLayout, writeDescriptorSet);
                vkCmdBindDescriptorSets(Engine::getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
            }

            ShaderVertexBuffer& vertexBuffer() {
                return perImageData[Engine::getCurrentSwapChainImage()].vertexBuffer;
            }
            ShaderIndexBuffer& indexBuffer() {
                return perImageData[Engine::getCurrentSwapChainImage()].indexBuffer;
            }
    };
}
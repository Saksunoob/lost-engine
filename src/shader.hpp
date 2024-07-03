#include <vulkan/vulkan.hpp>
#include "vulkan/pipeline.hpp"
#include "engine.hpp"


namespace engine {
    class Shader;

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
    struct ShaderVariable {
        ShaderVarType type;

        ShaderVariable(ShaderVarType type) : type(type) {};
    };
    class ShaderVertexBuffer {
        std::vector<ShaderVariable> variables;

        static unsigned getVarTypeSize(ShaderVarType type);

        public:
            ShaderVertexBuffer(std::vector<ShaderVariable> variables);
            ~ShaderVertexBuffer();
            /*
            ShaderVertexBuffer& operator=(ShaderVertexBuffer&) = delete;
            ShaderVertexBuffer& operator=(ShaderVertexBuffer&&) = delete;
            ShaderVertexBuffer(ShaderVertexBuffer&) = delete;
            ShaderVertexBuffer(ShaderVertexBuffer&&) = delete;
            */

            template <typename T>
            void bind(std::vector<T> bufferData) {
                set(bufferData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                VkDeviceSize offset[] = {0};
                vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, &buffers[currentBufferIndex], offset);
                currentBufferIndex++;
            }
            
            protected:
                VkVertexInputBindingDescription getBindingDescription();
                std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
                unsigned currentBufferIndex = 0;
                std::vector<VkBuffer> buffers;
                std::vector<VkDeviceMemory> memories;
                unsigned bindingSize = 0;

                template <typename T>
                void set(std::vector<T> bufferData, VkBufferUsageFlagBits usageFlag) {
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

                    VkDeviceSize bufferSize = sizeof(T) * bufferData.size();
                    device.createBuffer(bufferSize, usageFlag,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        buffer, memory);
                    void* data;
                    vkMapMemory(device.device(), memory, 0, bufferSize, 0, &data);
                    memcpy(data, bufferData.data(), static_cast<size_t>(bufferSize));
                    vkUnmapMemory(device.device(), memory);
                }

            friend Shader;
    };
    class ShaderIndexBuffer : public ShaderVertexBuffer {
        public:
            ShaderIndexBuffer() : ShaderVertexBuffer({ShaderVariable{VAR_UINT}}) {};
            void bind(std::vector<unsigned> indices) {
                set(indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                vkCmdBindIndexBuffer(Engine::getCurrentCommandBuffer(), buffers[currentBufferIndex], 0, VK_INDEX_TYPE_UINT32);
                currentBufferIndex++;
            }
    };

    class ShaderUniform {
        unsigned size;
        bool vertex, fragment;
        std::vector<VkBuffer> buffers;
        std::vector<VkDeviceMemory> memory;
        std::vector<void*> bufferMap;
        std::vector<VkDescriptorSet> descriptorSet;
    };

    class Shader {
        const char* shaderPath;

        Pipeline* pipeline;
        VkDescriptorSetLayout descriptorSetLayout;

        struct PerImageData {
            VkDescriptorPool descriptorPool;
            std::vector<ShaderUniform> uniforms;
            ShaderVertexBuffer vertexBuffer;
            ShaderIndexBuffer indexBuffer;

            PerImageData(std::vector<ShaderVariable> variables) : vertexBuffer(variables), indexBuffer() {};
        };

        std::vector<PerImageData> perImageData;

        public:
            Shader(const char* shaderPath, std::vector<ShaderVariable> variables, std::vector<ShaderUniform> uniforms);
            ~Shader();

            void recreate();
            void bind();

            ShaderVertexBuffer& vertexBuffer() {
                return perImageData[Engine::getCurrentSwapChainImage()].vertexBuffer;
            }
            ShaderIndexBuffer& indexBuffer() {
                return perImageData[Engine::getCurrentSwapChainImage()].indexBuffer;
            }
    };
}
#pragma once

#include <vulkan/vulkan.hpp>
#include "vulkan/pipeline.hpp"
#include "vulkan/device.hpp"
#include "engine.hpp"


namespace engine {
    class Shader;

    class DescriptorPool {
        const unsigned STARTING_POOL_SIZE = 16;

        static const std::vector<VkDescriptorType> types;

        std::vector<std::vector<VkDescriptorPool>> pools;

        struct Reserve {
            struct Sets {
                std::vector<VkDescriptorSet> sets; // One per set in pipeline

                void push(VkDescriptorSet set, unsigned set_index) {
                    if (sets.size() <= set_index) {
                        sets.resize(set_index+1);
                    }
                    sets[set_index] = set;
                }
            };
            
            unsigned bound;
            std::vector<Sets> sets; // One per draw call

            bool full() {
                return bound >= sets.size();
            }

            VkDescriptorSet current(unsigned set_index) {
                return sets.at(bound).sets.at(set_index);
            }

            void restart() {
                bound = 0;
                return;
            }

            void push(VkDescriptorSet set, unsigned set_index) {
                if (sets.size() <= bound) {
                    sets.push_back({});
                }
                sets[sets.size()-1].push(set, set_index);
                return;
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
        ~DescriptorPool();
        VkDescriptorSet writeDescriptor(VkDescriptorSetLayout set_layout, unsigned set_index, VkWriteDescriptorSet write);
        void bindDescriptorSet(Pipeline& pipeline, VkDescriptorSetLayout set_layout, unsigned set_index);
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
        std::vector<std::vector<ShaderVarType>> types;

        ShaderVariables(std::vector<std::vector<ShaderVarType>> types) : types(types) {};
        ShaderVariables(std::initializer_list<std::vector<ShaderVarType>> types) : types(types) {};

        unsigned getBindingSize(unsigned binding);
        unsigned getVariableSize(unsigned binding, unsigned index);

        std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    struct Binding {
        enum BindingType {
            BINDING_TYPE_UNIFORM,
            BINDING_TYPE_SAMPLER
        } type;
        unsigned size;

        static Binding Sampler() {
            return Binding{BINDING_TYPE_SAMPLER, 0};
        }
        static Binding Uniform(unsigned size) {
            return Binding{BINDING_TYPE_UNIFORM, size};
        }

        VkDescriptorType getType() {
            switch (type) {
                case BINDING_TYPE_UNIFORM:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case BINDING_TYPE_SAMPLER:
                    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            Logger::logError("Invalid binding type");
            throw;
        }
    };

    class UniformBuffer;
    class Buffer;
    class Texture;

    class Shader {
        const char* shaderPath;

        Pipeline* pipeline;
        unsigned pushConstantSize;
        std::vector<Binding> bindings;
        unsigned uniformCounter;

        VkDescriptorSetLayout descriptorSetLayout;

        static DescriptorPool* descriptorPool;

        ShaderVariables variables;
        std::vector<std::vector<std::unique_ptr<UniformBuffer>>> uniformBuffers;

        public:
            Shader(const char* shaderPath, ShaderVariables variables, unsigned pushConstantSize, std::vector<Binding> bindings);
            ~Shader();

            void recreate();
            void bind();

            void bindVertexBuffers(std::vector<Buffer*> buffers);

            void pushConstant(const void* data, unsigned size);
            void writeSamplerBinding(unsigned set, unsigned binding, Texture& texture);
            void writeUniformBinding(unsigned set, unsigned binding, const void* uniform);

            void bindSet(unsigned set);
    };
}
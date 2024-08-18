#include "shader.hpp"

namespace engine {

    ShaderVertexBuffer::ShaderVertexBuffer(std::vector<ShaderVariable> variables) : variables(variables) {
        for (unsigned i = 0; i < variables.size(); i++) {
            bindingSize += getVarTypeSize(variables[i].type);
        }
    }
    ShaderVertexBuffer::~ShaderVertexBuffer() {
        VkDevice device = Engine::getDevice().device();
        for (VkBuffer buffer : buffers) {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        for (VkDeviceMemory memory : memories) {
            vkFreeMemory(device, memory, nullptr);
        }
    }

    unsigned ShaderVertexBuffer::getVarTypeSize(ShaderVarType type) {
        switch (type)
        {
            case VAR_FLOAT:
                return 4;
            case VAR_INT:
                return 4;
            case VAR_UINT:
                return 4;
            case VAR_VEC2:
                return 8;
            case VAR_IVEC2:
                return 8;
            case VAR_UVEC2:
                return 8;
            case VAR_VEC3:
                return 12;
            case VAR_IVEC3:
                return 12;
            case VAR_UVEC3:
                return 12;
            case VAR_VEC4:
                return 16;
            case VAR_IVEC4:
                return 16;
            case VAR_UVEC4:
                return 16;
        }
    }

    VkVertexInputBindingDescription ShaderVertexBuffer::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescriptions;
        bindingDescriptions.binding = 0;
        bindingDescriptions.stride = bindingSize;
        bindingDescriptions.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> ShaderVertexBuffer::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(variables.size());

        unsigned offset = 0;

        for (unsigned i = 0; i < variables.size(); i++) {
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].format = static_cast<VkFormat>(variables[i].type);
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].offset = offset;

            offset += getVarTypeSize(variables[i].type);
        }
        

        return attributeDescriptions;
    }

    Shader::Shader(const char* shaderPath, std::vector<ShaderVariable> variables) :
        shaderPath(shaderPath) {

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;

        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(Engine::getDevice().device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            Logger::logError("failed to create descriptor set layout!");
            return;
        }

        for (int i = 0; i < Engine::getSwapChain()->imageCount(); i++) {
            perImageData.push_back(PerImageData(variables));
        }
        recreate();
    }

    Shader::~Shader() {
        VkDevice device = Engine::getDevice().device();
        Pipeline* pipeline;

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;

        delete pipeline;
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }

    void Shader::recreate() {
        Device& device = Engine::getDevice();

        auto pipelineConfig = PipelineConfig::defaultConfig(Engine::getSwapChain()->width(), Engine::getSwapChain()->height());
        pipelineConfig.renderPass = Engine::getSwapChain()->getRenderPass();
        pipelineConfig.pipelineLayoutInfo.setLayoutCount = 1;

        pipelineConfig.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineConfig.attributeDescriptions = vertexBuffer().getAttributeDescriptions();
        pipelineConfig.bindingDescriptions = {vertexBuffer().getBindingDescription()};
        delete pipeline;
        pipeline = new Pipeline(Engine::getDevice(), shaderPath, pipelineConfig);
        
        // Change to non coantant
        VkDeviceSize bufferSize = sizeof(glm::mat4)*2;
    }

    void Shader::bind() {
        if (Engine::wasWindowResized()) {
            recreate();
        }
        pipeline->bind(Engine::getCurrentCommandBuffer());
        vertexBuffer().currentBufferIndex = 0;
        indexBuffer().currentBufferIndex = 0;
    }
}
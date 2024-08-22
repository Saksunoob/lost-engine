#include "shader.hpp"

namespace engine {

    inline unsigned DescriptorPool::getPoolIndex() {
        return std::log2(currently_allocated/STARTING_POOL_SIZE+1);
    }

    void DescriptorPool::createDescriptorPool(std::vector<VkDescriptorPool>& pool) {
        unsigned size = (1 << pool.size()) * STARTING_POOL_SIZE;

        VkDescriptorPoolSize pool_size = {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            size
        };

        VkDescriptorPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.maxSets = size;
        createInfo.poolSizeCount = 1;
        createInfo.pPoolSizes = &pool_size;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        VkDescriptorPool descriptorPool;

        if (vkCreateDescriptorPool(Engine::getDevice().device(), &createInfo, nullptr, &descriptorPool)) {
            Logger::logError("Failed to create descriptor pool");
        }

        pool.push_back(descriptorPool);
        return;
    }

    DescriptorPool::DescriptorPool() : pools(Engine::getSwapChain()->imageCount()), pool_size(STARTING_POOL_SIZE), reserves(Engine::getSwapChain()->imageCount()) {
        for (unsigned i = 0; i < pools.size(); i++) {
            createDescriptorPool(pools[i]);
        }
    }

    VkDescriptorSet DescriptorPool::writeDescriptorSet(VkDescriptorSetLayout set_layout, VkWriteDescriptorSet write) {
        std::vector<VkDescriptorPool>& framePools = pools[Engine::getCurrentSwapChainImage()];

        if (Engine::getCurrentSwapChainImage() != last_image) {
            for (auto& [set, reserve] : reserves[Engine::getCurrentSwapChainImage()]) {
                reserve.written = 0;
            }
            last_image = Engine::getCurrentSwapChainImage();
        }

        Reserve& reserve = reserves[Engine::getCurrentSwapChainImage()][set_layout];

        if (!reserve.full()) {
            VkDescriptorSet set = reserve.next();
            write.dstSet = set;
            vkUpdateDescriptorSets(Engine::getDevice().device(), 1, &write, 0, nullptr);
            return set;
        }

        unsigned index = getPoolIndex();

        while (index >= framePools.size()) {
            createDescriptorPool(framePools);
        }

        VkDescriptorSetAllocateInfo setAllocInfo;
        setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocInfo.descriptorSetCount = 1;
        setAllocInfo.descriptorPool = framePools[index];
        setAllocInfo.pSetLayouts = &set_layout;
        setAllocInfo.pNext = nullptr;

        VkDescriptorSet descriptor_set;
        if (vkAllocateDescriptorSets(Engine::getDevice().device(), &setAllocInfo, &descriptor_set)) {
            Logger::logError("Failed to allocate descriptor set");
        }
        currently_allocated += 1;
        reserve.sets.push_back(descriptor_set);
        
        write.dstSet = descriptor_set;
        vkUpdateDescriptorSets(Engine::getDevice().device(), 1, &write, 0, nullptr);
        reserve.written += 1;
        return descriptor_set;
    }

    unsigned ShaderVariables::getTotalSize() {
        unsigned size = 0;
        for (int i = 0; i < types.size(); i++) {
            size += getVariableSize(i);
        }
        return size;
    }

    unsigned ShaderVariables::getVariableSize(unsigned index) {
        switch (types[index])
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

    std::vector<VkVertexInputBindingDescription> ShaderVariables::getBindingDescriptions() {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = getTotalSize();
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return {bindingDescription};
    }
    std::vector<VkVertexInputAttributeDescription> ShaderVariables::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(types.size());
        unsigned offset = 0;
        for (unsigned i = 0; i < types.size(); i++) {
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].format = static_cast<VkFormat>(types[i]);
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].offset = offset;

            offset += getVariableSize(i);
        }
        return attributeDescriptions;
    }

    DescriptorPool* Shader::descriptorPool = nullptr;

    Shader::Shader(const char* shaderPath, ShaderVariables variables, unsigned pushConstantSize, unsigned uniformSize) :
        shaderPath(shaderPath), pushConstantSize(pushConstantSize), uniformSize(uniformSize), variables(variables), uniformBuffers(Engine::getSwapChain()->imageCount()) {

        if (descriptorPool == nullptr) {
            descriptorPool = new DescriptorPool();
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

        VkDescriptorSetLayoutBinding uniformBufferBinding = {};
        uniformBufferBinding.binding = 0;
        uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBufferBinding.descriptorCount = 1;
        uniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uniformBufferBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.flags = 0;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pNext = nullptr;
        layoutInfo.pBindings = &uniformBufferBinding;

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantSize;

        auto pipelineConfig = PipelineConfig::defaultConfig(Engine::getSwapChain()->width(), Engine::getSwapChain()->height());
        pipelineConfig.renderPass = Engine::getSwapChain()->getRenderPass();
        pipelineConfig.pipelineLayoutInfo.pushConstantRangeCount = pushConstantSize != 0;
        pipelineConfig.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        pipelineConfig.pipelineLayoutInfo.setLayoutCount = 1;
        pipelineConfig.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineConfig.attributeDescriptions = variables.getAttributeDescriptions();
        pipelineConfig.bindingDescriptions = variables.getBindingDescriptions();
        delete pipeline;
        pipeline = new Pipeline(Engine::getDevice(), shaderPath, pipelineConfig);
    }

    void Shader::bind() {
        if (Engine::wasWindowResized()) {
            recreate();
        }
        pipeline->bind(Engine::getCurrentCommandBuffer());
        uniformBuffers[Engine::getCurrentSwapChainImage()].clear();
    }

    void Shader::pushConstant(const void* data, unsigned size) {
        vkCmdPushConstants(Engine::getCurrentCommandBuffer(), pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
    }

    void Shader::bindUniform(const void* uniform) {
        std::vector<std::unique_ptr<UniformBuffer>>& buffers = uniformBuffers[Engine::getCurrentSwapChainImage()];
        buffers.emplace_back(std::make_unique<UniformBuffer>(uniformSize));
        UniformBuffer& buffer = *buffers.at(buffers.size()-1).get();

        buffer.setVector(uniform, 1);
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffer.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = uniformSize;

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
}
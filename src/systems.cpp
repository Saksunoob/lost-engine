#include "systems.hpp"

static std::unique_ptr<engine::Pipeline> pipeline{};
static std::vector<VkBuffer> uniformBuffers;
static std::vector<VkDeviceMemory> uniformBuffersMemory;
static std::vector<void*> uniformBuffersMapped;

static VkDescriptorPool descriptorPool = nullptr;
static VkDescriptorSetLayout descriptorSetLayout;
std::vector<VkDescriptorSet> descriptorSets;

void engine::renderMeshes(Scene& scene) {
    Device& device = Engine::getDevice();

    if (!pipeline || Engine::wasWindowResized()) {
        auto pipelineConfig = PipelineConfig::defaultConfig(Engine::getSwapChain()->width(), Engine::getSwapChain()->height());
        pipelineConfig.renderPass = Engine::getSwapChain()->getRenderPass();
        pipelineConfig.pipelineLayoutInfo.setLayoutCount = 1;

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;

        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            Logger::logError("failed to create descriptor set layout!");
            return;
        }
        pipelineConfig.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineConfig.attributeDescriptions = Mesh::getAttributeDescriptions();
        pipelineConfig.bindingDescriptions = Mesh::getBindingDescriptions();

        pipeline = std::make_unique<Pipeline>(Engine::getDevice(), "src/shaders/Mesh", pipelineConfig);

        VkDeviceSize bufferSize = sizeof(glm::mat4)*2;

        uniformBuffers.resize(Engine::getSwapChain()->imageCount()*2);
        uniformBuffersMemory.resize(Engine::getSwapChain()->imageCount()*2);
        uniformBuffersMapped.resize(Engine::getSwapChain()->imageCount()*2);

        for (size_t i = 0; i < Engine::getSwapChain()->imageCount()*2; i++) {
            device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device.device(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    if (descriptorPool == nullptr) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(Engine::getSwapChain()->imageCount()*2);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(Engine::getSwapChain()->imageCount()*2);

        if (vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            Logger::logError("Failed to create descriptor pool");
            return;
        }

        std::vector<VkDescriptorSetLayout> layouts(Engine::getSwapChain()->imageCount()*2, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(Engine::getSwapChain()->imageCount())*2;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(Engine::getSwapChain()->imageCount()*2);
        if (vkAllocateDescriptorSets(device.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            Logger::logError("failed to allocate descriptor sets!");
            return;
        }
    }


    pipeline->bind(Engine::getCurrentCommandBuffer());

    Components cameras = scene.getComponent<Camera>();
    Components transforms = scene.getComponent<GlobalTransform>();
    
    Entity main_camera;
    for (unsigned i = 0; i < cameras.size(); i++) {
        if (cameras[i] != nullptr && cameras[i]->main) {
            if (transforms[i] != nullptr) {
                main_camera = i;
                break;
            }
            Logger::logWarning("Main camera has no GlobalTransform component");            
        }
        if (i+1 == cameras.size()) {
            Logger::logWarning("No main camera");
            return;
        }
    }
    
    Components meshes = scene.getComponent<Mesh>();
    Components textures = scene.getComponent<Texture>();

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    unsigned index = 0;
    
    for (unsigned i = 0; i < transforms.size(); i++) {
        if (transforms[i] != nullptr && meshes[i] != nullptr && textures[i] != nullptr) {
            Mesh* mesh = meshes[i];

            if (mesh->vertexBuffer == nullptr) {
                mesh->createBuffers(device);
            }

            VkBuffer buffers[] = {mesh->vertexBuffer};
            VkDeviceSize offset[] = {0};
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offset);
            vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            struct Matrices {
                glm::mat4 trans;
                glm::mat4 proj;
            } matrices;

            matrices.trans = transforms[i]->getTransformationMatrix();
            matrices.proj = Camera::getProjectionMatrix(*transforms[main_camera], Engine::getWindowSize());
            memcpy(uniformBuffersMapped[Engine::getCurrentSwapChainImage()*2+index], &matrices, sizeof(matrices));

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[Engine::getCurrentSwapChainImage()*2+index];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(matrices);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[Engine::getCurrentSwapChainImage()*2+index];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional
            vkUpdateDescriptorSets(device.device(), 1, &descriptorWrite, 0, nullptr);
            
            vkCmdBindDescriptorSets(
                cmdBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipeline->getPipelineLayout(), 
                0, 1, 
                &descriptorSets[Engine::getCurrentSwapChainImage()*2+index],
                0, nullptr
            );

            vkCmdDrawIndexed(Engine::getCurrentCommandBuffer(), mesh->indices.size(), 1, 0, 0, 0);

            index++;
        }
    }
}
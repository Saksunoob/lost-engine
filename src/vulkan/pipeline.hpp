#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace engine {
    struct PipelineConfig {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;

        static PipelineConfig defaultConfig(unsigned width, unsigned height);
    };

    class Pipeline
    {
        Device& device;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkShaderModule vertexModule;
        VkShaderModule fragmentModule;
        const PipelineConfig config;

        void createShaderModule(const std::string code, VkShaderModule* PipelineModule);

    public:
        Pipeline(const Pipeline&) = delete;
        void operator=(const Pipeline&) = delete;

        // constructor reads and builds the Pipeline
        Pipeline(Device& device, const char* dirPath, PipelineConfig& config);
        Pipeline() = default;
        ~Pipeline();

        // use/activate the Pipeline
        void bind(VkCommandBuffer commandBuffer);
        VkPipelineLayout getPipelineLayout() {return pipelineLayout;};
    };
}

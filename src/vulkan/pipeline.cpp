#include "pipeline.hpp"

using namespace engine;

PipelineConfig PipelineConfig::defaultConfig(unsigned width, unsigned height) {
    PipelineConfig config{};

    config.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    config.viewport.x = 0.0f;
    config.viewport.y = 0.0f;
    config.viewport.width = static_cast<float>(width);
    config.viewport.height = static_cast<float>(height);
    config.viewport.minDepth = 0.0f;
    config.viewport.maxDepth = 1.0f;
    
    config.scissor.offset = {0, 0};
    config.scissor.extent = {width, height};
    
    config.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterizationInfo.depthClampEnable = VK_FALSE;
    config.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    config.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterizationInfo.lineWidth = 1.0f;
    config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    config.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterizationInfo.depthBiasEnable = VK_FALSE;
    config.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    config.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    config.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
    
    config.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisampleInfo.sampleShadingEnable = VK_FALSE;
    config.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config.multisampleInfo.minSampleShading = 1.0f;           // Optional
    config.multisampleInfo.pSampleMask = nullptr;             // Optional
    config.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    config.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    
    config.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    config.colorBlendAttachment.blendEnable = VK_FALSE;
    config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
    
    config.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config.colorBlendInfo.logicOpEnable = VK_FALSE;
    config.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    config.colorBlendInfo.attachmentCount = 1;
    config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;
    config.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    config.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    config.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    config.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
    
    config.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depthStencilInfo.depthTestEnable = VK_TRUE;
    config.depthStencilInfo.depthWriteEnable = VK_TRUE;
    config.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    config.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    config.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    config.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    config.depthStencilInfo.stencilTestEnable = VK_FALSE;
    config.depthStencilInfo.front = {};  // Optional
    config.depthStencilInfo.back = {};   // Optional

    return config;
}

Pipeline::~Pipeline() {
    vkDestroyShaderModule(device.device(), vertexModule, nullptr);
    vkDestroyShaderModule(device.device(), fragmentModule, nullptr);

    vkDestroyPipeline(device.device(), pipeline, nullptr);
}

Pipeline::Pipeline(Device& device, const char* dirPath, PipelineConfig& config) : device(device), config(config) {
    std::string vertexPath(dirPath);
    std::string fragmentPath(dirPath);

    vertexPath.append("/shader.vert.spv");
    fragmentPath.append("/shader.frag.spv");

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    createPipelineLayout();
    createShaderModule(vertexCode, &vertexModule);
    createShaderModule(fragmentCode, &fragmentModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto bindingDescriptions = Vertex::getBindingDescriptions();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned>(attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<unsigned>(bindingDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &config.viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &config.scissor;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &config.rasterizationInfo;
    pipelineInfo.pMultisampleState = &config.multisampleInfo;
    pipelineInfo.pColorBlendState = &config.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = config.subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        Logger::logError("Unable to create graphics pipeline");
        throw;
    }
}

void Pipeline::createShaderModule(const std::string code, VkShaderModule* shaderModule) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const unsigned*>(code.data());

    if (vkCreateShaderModule(device.device(), &create_info, nullptr, shaderModule) != VK_SUCCESS) {
        Logger::logError("Failed to create shadermodule for shader: "+code);
    }
}

void Pipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;

    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
        Logger::logError("Failed to create pipeline layout");
    } 
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
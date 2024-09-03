#include "components.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/device.hpp"
#include "buffer.hpp"
#include "engine.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


using namespace engine;

EntityComponents Components::operator[](unsigned index) {
    return EntityComponents(*this, index);
}

glm::mat4 Transform::getTransformationMatrix() const {
    glm::mat4 matrix = glm::mat4(1);
    matrix = glm::translate(matrix, glm::vec3(position.x, position.y, 0.0));
    matrix = glm::rotate(matrix, (float)rotation, glm::vec3(0, 0, 1));
    matrix = glm::scale(matrix, glm::vec3(scale.x, scale.y, 1.0));
    
    return matrix;
}

glm::mat4 Transform::getTransformationMatrix(float z) const {
    glm::mat4 matrix = glm::mat4(1);
    matrix = glm::translate(matrix, glm::vec3(position.x, position.y, z));
    matrix = glm::rotate(matrix, (float)rotation, glm::vec3(0, 0, 1));
    matrix = glm::scale(matrix, glm::vec3(scale.x, scale.y, 1.0));
    
    return matrix;
}

int ZLayer::min_layer = 0;
int ZLayer::max_layer = 0;

ZLayer::ZLayer(int layer, float order) : layer(layer), order(order) {
    if (order < 0. || order >= 1.) {
        Logger::logWarning("ZLayer.order should be kept at 0 <= order < 1. It is set at: " + std::to_string(order));
    }

    min_layer = std::min(layer, min_layer);
    max_layer = std::max(layer, max_layer);
}

void ZLayer::setLayer(int new_layer) {
    layer = new_layer;

    min_layer = std::min(layer, min_layer);
    max_layer = std::max(layer, max_layer);
}

float ZLayer::getZ() {
    if (order < 0. || order >= 1.) {
        Logger::logWarning("ZLayer.order should be kept at 0 <= order < 1. It is set at: " + std::to_string(order));
    }

    int layer_count = max_layer-min_layer+1;
    float layer_width = 1./layer_count;
    int rel_layer = layer-min_layer;

    return rel_layer*layer_width+order*layer_width;
}

glm::mat4 Camera::getProjectionMatrix(const Transform* transform, IVector2 window_size) {
    glm::mat4 matrix(1.0);
    if (transform) {
        transform->getTransformationMatrix();
    }
    matrix = glm::scale(matrix, glm::vec3(window_size.x/2.0, window_size.y/2.0, 1.0));

    return glm::inverse(matrix);
}

TextureData::TextureData(const void* data, IVector2 size, TextureFormat format) : size(size), format(format) {
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;

    StagingBuffer stagingBuffer(format.channels);
    stagingBuffer.setVector(data, size.x*size.y);

    imageFormat = format.getFormat();

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = imageFormat;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.extent = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1};
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    Device& device = Engine::getDevice();

    device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    device.copyBufferToImage(stagingBuffer.buffer, image, static_cast<uint>(size.x), static_cast<uint>(size.y), 1);

    generateMipmaps();
    imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.maxAnisotropy = 4.0;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler);

    VkImageViewCreateInfo imageViewInfo {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = imageFormat;
    imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.image = image;

    vkCreateImageView(device.device(), &imageViewInfo, nullptr, &imageView);
}

TextureData::TextureData(TextureData &&other) : size(other.size), mipLevels(other.mipLevels),
    image(other.image), imageMemory(other.imageMemory), imageView(other.imageView), sampler(other.sampler), imageFormat(other.imageFormat), imageLayout(other.imageLayout) {
    other.image = nullptr;
    other.imageMemory = nullptr;
    other.imageView = nullptr;
    other.sampler = nullptr;
}

TextureData::~TextureData() {
    Device& device = Engine::getDevice();
    if (image) {
        vkDestroyImage(device.device(), image, nullptr);
    }
    if (imageMemory) {
        vkFreeMemory(device.device(), imageMemory, nullptr);
    }
    if (imageView) {
        vkDestroyImageView(device.device(), imageView, nullptr);
    }
    if (sampler) {
        vkDestroySampler(device.device(), sampler, nullptr);
    }
}

void TextureData::update(const void* data) {
    transitionImageLayout(imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    StagingBuffer stagingBuffer(format.channels);
    stagingBuffer.setVector(data, size.x * size.y);

    Device& device = Engine::getDevice();
    device.copyBufferToImage(stagingBuffer.buffer, image, static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1);

    generateMipmaps();
}

void TextureData::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    Device& device = Engine::getDevice();
    VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else {
        throw std::runtime_error("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    device.endSingleTimeCommands(commandBuffer);
}

Texture::Texture(const std::string &filepath) {
    int channels;
    int m_BytesPerPixel;
    IVector2 size;
    auto img_data = stbi_load(filepath.c_str(), &size.x, &size.y, &m_BytesPerPixel, 4);
    data = std::shared_ptr<TextureData>(new TextureData(img_data, size, TextureFormat::Srgb(4)));

    stbi_image_free(img_data);
}

Texture::Texture(const void* img_data, IVector2 size, TextureFormat format) {
    data = std::shared_ptr<TextureData>(new TextureData(img_data, size, format));
}

void TextureData::generateMipmaps() {
    Device& device = Engine::getDevice();
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = size.x;
    int32_t mipHeight = size.y;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    device.endSingleTimeCommands(commandBuffer);
}

TileMap::TileMap(IVector2 size) : size(size), tiles(size.x*size.y,-1) {};

void TileMap::setTile(IVector2 pos, int value) {
    tiles.at(pos.y*size.x+pos.x) = value;
}

void UITransform::calculateAbsolute(IVector2 window_size, Entity entity) {
    Transform root({0, 0}, {0, 0}, 0);
    UITransform* parent = entity.getParent().getComponent<UITransform>();
    if (parent != nullptr) {
        root = parent->absolute;
    } else {
        root.scale = Vector2(window_size.x, window_size.y);
    }

    switch (position_type) {
        case UNIT_PERCENT:
            absolute.position = root.position + (position/100*root.scale).rotate(root.rotation);
            break;
        case UNIT_PIXELS:
            absolute.position = root.position + position.rotate(root.rotation);
            break;
    }

    if (size_type == UNIT_PERCENT) {
        absolute.scale = root.scale * (size/100.);
    } else if (position_type == UNIT_PIXELS) {
        absolute.scale = size;
    }

    absolute.rotation = root.rotation + rotation;
}

Transform UITransform::getAbsoute() {
    return absolute;
}
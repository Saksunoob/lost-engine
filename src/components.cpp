#include "components.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/device.hpp"
#include "buffer.hpp"
#include "engine.hpp"
#include "resources.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


using namespace engine;

Entity Entites::operator[](unsigned index) {
    return Entity(filter[index]+1, scene);
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

Transform Transform::operator*(const Transform& other) const {
    Transform trans{Vector2{}, Vector2{}, 0};

    trans.position = other.position + (position * other.scale).rotate(other.rotation);
    trans.scale = scale * other.scale;
    trans.rotation = rotation + other.rotation;

    return trans;
}

void GlobalTransform::operator=(const Transform& other) {
    position = other.position;
    scale = other.scale;
    rotation = other.rotation;
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

glm::mat4 Camera::getProjectionMatrix(const Transform* transform) {
    IVector2 window_size = Engine::getWindowSize();
    glm::mat4 matrix(1.0);
    if (transform) {
        transform->getTransformationMatrix();
    }
    matrix = glm::scale(matrix, glm::vec3(window_size.x/2.0, window_size.y/2.0, 1.0));

    return glm::inverse(matrix);
}

Vector2 Camera::screenToWorldPos(const Transform* transform, IVector2 screen_pos) {
    Vector2 pos = screen_pos - Engine::getWindowSize()/2;
    glm::vec4 projected_pos = glm::vec4(pos.x, pos.y, 0.0, 1.0) * getProjectionMatrix(transform);
    return Vector2(projected_pos.x, projected_pos.y);
}
IVector2 Camera::worldToScreenPos(const Transform* transform, Vector2 world_pos) {
    glm::vec4 projected_pos = glm::vec4(world_pos.x, world_pos.y, 0.0, 1.0) * glm::inverse(getProjectionMatrix(transform));
    IVector2 pos(projected_pos.x, projected_pos.y);
    return pos + Engine::getWindowSize()/2;
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
    image(other.image), imageMemory(other.imageMemory), imageView(other.imageView), imageFormat(other.imageFormat), imageLayout(other.imageLayout) {
    other.image = nullptr;
    other.imageMemory = nullptr;
    other.imageView = nullptr;
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

std::unordered_map<std::string, std::shared_ptr<TextureData>> Texture::texture_files = {};

Texture::Texture(const std::string &filepath, Filter filter, AddressMode address_mode, bool mipmaps, Filter mipmap_filter) {
    if (texture_files.find(filepath) == texture_files.end()) {
        int channels;
        int m_BytesPerPixel;
        auto img_data = stbi_load(filepath.c_str(), &size.x, &size.y, &m_BytesPerPixel, 4);
        texture_files[filepath] = std::shared_ptr<TextureData>(new TextureData(img_data, size, TextureFormat::Srgb(4)));

        stbi_image_free(img_data);
    }
    
    data = texture_files[filepath];
    createSampler (filter, address_mode, mipmaps, mipmap_filter);
}

Texture::Texture(const void* img_data, IVector2 size, TextureFormat format, Filter filter, AddressMode address_mode, bool mipmaps, Filter mipmap_filter) : size(size) {
    data = std::shared_ptr<TextureData>(new TextureData(img_data, size, format));
    createSampler (filter, address_mode, mipmaps, mipmap_filter);
}

void Texture::createSampler(Filter filter, AddressMode address_mode, bool mipmaps, Filter mipmap_filter) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    samplerInfo.magFilter = (VkFilter)filter;
    samplerInfo.minFilter = (VkFilter)filter;
    switch (mipmap_filter) {
        case Filter::LINEAR:
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case Filter::NEAREST:
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
    }
    samplerInfo.addressModeU = (VkSamplerAddressMode)address_mode;
    samplerInfo.addressModeV = (VkSamplerAddressMode)address_mode;
    samplerInfo.addressModeW = (VkSamplerAddressMode)address_mode;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = mipmaps ? static_cast<float>(data->getMipLevels()) : 0.0f;
    samplerInfo.maxAnisotropy = 4.0;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VkSampler new_sampler;
    vkCreateSampler(Engine::getDevice().device(), &samplerInfo, nullptr, &new_sampler);

    sampler = std::make_shared<VkSampler>(new_sampler);
}

Texture::~Texture() {
    if (sampler.unique()) {
        vkDestroySampler(Engine::getDevice().device(), *sampler.get(), nullptr);
    }
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

void UITransform::calculateGlobal(IVector2 window_size, Entity entity) {
    Transform root({0, 0}, {0, 0}, 0);
    Entity parent = entity.getParent();
    if (!parent.isNull()) {
        root = *parent.getComponent<GlobalTransform>();
    } else {
        root.scale = Vector2(window_size.x, window_size.y);
    }

    GlobalTransform& global = *entity.getComponent<GlobalTransform>();

    switch (size_type) {
        case UNIT_PERCENT:
            global.scale = root.scale * (size/100.);
            break;
        case UNIT_PIXELS:
            global.scale = size;
    }

    global.rotation = root.rotation + rotation;

    Vector2 anchor_mod{0, 0};

    switch (anchor.horizontal) {
        case POINT_LEFT:
            anchor_mod.x -= root.scale.x/2.;
            break;
        case POINT_RIGHT:
            anchor_mod.x += root.scale.x/2.;
            break;
    }
    switch (anchor.vertical) {
        case POINT_TOP:
            anchor_mod.y -= root.scale.y/2.;
            break;
        case POINT_BOTTOM:
            anchor_mod.y += root.scale.y/2.;
            break;
    }

    root.position = root.position + anchor_mod.rotate(root.rotation);

    Vector2 origin_mod{0, 0};
    switch (origin.horizontal) {
        case POINT_LEFT:
            origin_mod.x += global.scale.x/2.;
            break;
        case POINT_RIGHT:
            origin_mod.x -= global.scale.x/2.;
            break;
    }
    switch (origin.vertical) {
        case POINT_TOP:
            origin_mod.y += global.scale.y/2.;
            break;
        case POINT_BOTTOM:
            origin_mod.y -= global.scale.y/2.;
            break;
    }

    root.position = root.position + origin_mod.rotate(global.rotation);

    switch (position_type) {
        case UNIT_PERCENT:
            global.position = root.position + (position/100*root.scale).rotate(root.rotation);
            break;
        case UNIT_PIXELS:
            global.position = root.position + position.rotate(root.rotation);
            break;
    }
}

float area(const Vector2& A, const Vector2& B, const Vector2& C) {
    return 0.5 * std::abs(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
}

// Function to check if two floating-point numbers are approximately equal
bool approximatelyEqual(float a, float b) {
    return std::abs(a - b) < FLT_EPSILON;
}

// Function to check if the point P is inside the triangle formed by points A, B, and C
bool isPointInTriangle(const Vector2& A, const Vector2& B, const Vector2& C, const Vector2& P) {
    float totalArea = area(A, B, C);
    float areaPAB = area(P, A, B);
    float areaPBC = area(P, B, C);
    float areaPCA = area(P, C, A);

    // Check if the sum of the areas of the sub-triangles is approximately equal to the area of the whole triangle
    return approximatelyEqual(totalArea, areaPAB + areaPBC + areaPCA);
}

bool Collider::collidesWithPoint(Vector2 point, Entity entity) {
    Transform& transform = *entity.getComponent<GlobalTransform>();
    
    switch (type) {
        case ColliderType::CIRCLE:
            return transform.position.distance(point) >= data.radius;
        case ColliderType::SQUARE: {
            Transform abs_transform = transform;
            Vector2 rel_point = (point - abs_transform.position).rotate(-abs_transform.rotation) / abs_transform.scale;
            return data.square.collidesWithPoint(rel_point);
        }
        case ColliderType::MESH: {
            Transform abs_transform = transform;
            Vector2 rel_point = (point - abs_transform.position).rotate(-abs_transform.rotation) / abs_transform.scale;
            Vertices* vertices = entity.getComponent<Vertices>();
            Indices* indices = entity.getComponent<Indices>();
            if (!vertices) {
                Logger::logWarning("No mesh provided for Mesh collider");
                return false;
            }
            const std::vector<Vector2>& vert_vec = vertices->getData();
            if (indices) {
                const std::vector<unsigned>& idx_vec = indices->getData();
                for (int i = 0; i < idx_vec.size()/3; i++) {
                    if (isPointInTriangle(vert_vec[idx_vec[i]], vert_vec[idx_vec[i+1]], vert_vec[idx_vec[i+2]], rel_point)) {
                        return true;
                    }
                }
                return false;
            }
            for (int i = 0; i < vert_vec.size()/3; i++) {
                if (isPointInTriangle(vert_vec[i], vert_vec[i+1], vert_vec[i+2], rel_point)) {
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

std::vector<Vector2> getAxes(Polygon poly) {
    std::vector<Vector2> axes(poly.points.size());
    for (int i = 0; i < poly.points.size(); i++) {
        Vector2 edge = poly.points[i] - poly.points[(i+1)%poly.points.size()];
        Vector2 normal = edge.normal();
        axes[i] = (normal / normal.magnitude());
    }
    return axes;
}

std::array<float, 2> project(Polygon poly, Vector2 axis) {
    float dot = poly.points[0].dot(axis);
    float min = dot;
    float max = dot;
    for (int i = 1; i < poly.points.size(); i++) {
        float dot = poly.points[i].dot(axis);
        min = std::min(min, dot);
        max = std::max(max, dot);
    }
    return {min, max};
}

bool polygonsCollide(Polygon poly1, Polygon poly2) {
    std::vector<Vector2> axes = getAxes(poly1);
    std::vector<Vector2> axes2 = getAxes(poly2);
    axes.insert(axes.end(), axes2.begin(), axes2.end());

    for (Vector2 axis : axes) {
        std::array<float, 2> proj1 = project(poly1, axis);
        std::array<float, 2> proj2 = project(poly2, axis);

        if (proj1[1] < proj2[0] || proj2[1] < proj1[0]) {
            return false;
        }
    }
    return true;
}

std::vector<Polygon> Collider::ColliderInfo::getPolygons() {
    if (collider.type == ColliderType::SQUARE) {
        return {{collider.data.square.getPolygon().transformed(transform)}};
    }
    if (collider.type == ColliderType::MESH) {
        const std::vector<Vector2>& vert_vec = vertices->getData();
        if (indices) {
            std::vector<Polygon> polygons(indices->item_count/3);
            const std::vector<unsigned>& idx_vec = indices->getData();
            for (int i = 0; i < idx_vec.size()/3; i++) {
                polygons[i] = Polygon{{vert_vec[idx_vec[i]], vert_vec[idx_vec[i+1]], vert_vec[idx_vec[i+2]]}}.transformed(transform);
            }
            return polygons;
        }
        std::vector<Polygon> polygons(vertices->item_count/3);
        for (int i = 0; i < vert_vec.size()/3; i++) {
            polygons[i] = Polygon{{vert_vec[i], vert_vec[i+1], vert_vec[i+2]}}.transformed(transform);
        }
        return polygons;
    }
    return {};
}

bool Collider::collidesWith(ColliderInfo other, Entity entity) {
    Transform& transform = *entity.getComponent<GlobalTransform>();

    if (type != ColliderType::CIRCLE && other.collider.type != ColliderType::CIRCLE) {
        Vertices* vertices = entity.getComponent<Vertices>();
        Indices* indices = entity.getComponent<Indices>();

        std::vector<Polygon> polygons1 = ColliderInfo{*this, transform, vertices, indices}.getPolygons();
        std::vector<Polygon> polygons2 = other.getPolygons();

        for (Polygon& p1 : polygons1) {
            for (Polygon& p2 : polygons2) {
                if (polygonsCollide(p1, p2)) {
                    return true;
                }
            }
        }
        return false;
    }
    if (type == ColliderType::CIRCLE && other.collider.type == ColliderType::CIRCLE) {
        Vector2 p1 = transform.position;
        Vector2 p2 = transform.position;
        float r1 = transform.scale.magnitude()/sqrt(2) * data.radius;
        float r2 = transform.scale.magnitude()/sqrt(2) * other.collider.data.radius;

        return p1.distance(p2) <= r1+r2;
    }
    Logger::logWarning("Unimplemented collision");
    return false;
}

bool engine::Collider::hovering(Entity entity) {
    return collidesWithPoint(entity.scene.getResource<Input>().getUIMousePos(), entity);
}

// Must be called every frame to work properly
bool engine::Collider::clicked(uint8_t button, Entity entity) {
    Input& input = entity.scene.getResource<Input>();
    if (input.getMouseButtonJustPressed(button)) {
        mouse_down_on_this = hovering(entity);
        return false;
    }
    if (input.getMouseButtonJustReleased(button)) {
        if (hovering(entity) && mouse_down_on_this) {
            mouse_down_on_this = false;
            return true;
        }
        mouse_down_on_this = false;
        return false;
    }
    return false;
}

Collider::ColliderInfo Collider::getInfo(Entity entity) {
    Transform& transform = *entity.getComponent<GlobalTransform>();
    return ColliderInfo {
        collider: *this,
        transform: transform,
        vertices: entity.getComponent<Vertices>(),
        indices: entity.getComponent<Indices>()
    };
}

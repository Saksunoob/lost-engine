#include "components.hpp"

using namespace engine;

void Mesh::createBuffers(Device& device) {
    _device = &device;

    // Vertex buffer
    VkDeviceSize vertexbufferSize = sizeof(vertices[0]) * vertices.size();
    device.createBuffer(vertexbufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer, vertexBufferMemory);

    void* data;
    vkMapMemory(device.device(), vertexBufferMemory, 0, vertexbufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(vertexbufferSize));
    vkUnmapMemory(device.device(), vertexBufferMemory);
    
    // Index buffer
    VkDeviceSize indexbufferSize = sizeof(indices[0]) * indices.size();
    device.createBuffer(indexbufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer, indexBufferMemory);

    vkMapMemory(device.device(), indexBufferMemory, 0, indexbufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(indexbufferSize));
    vkUnmapMemory(device.device(), indexBufferMemory);
}

std::vector<VkVertexInputBindingDescription> Mesh::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vector2);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Mesh::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].offset = 0;

    return attributeDescriptions;
}

glm::mat4 Transform::getTransformationMatrix() const {
    glm::mat4 matrix = glm::mat4(1);
    matrix = glm::translate(matrix, glm::vec3(position.x, position.y, 0.0));
    matrix = glm::rotate(matrix, (float)rotation, glm::vec3(0, 0, 1));
    matrix = glm::scale(matrix, glm::vec3(scale.x, scale.y, 1.0));
    
    return matrix;
}

glm::mat4 Camera::getProjectionMatrix(const Transform& transform, IVector2 window_size) {
    int viewport[4];
    glm::mat4 matrix = transform.getTransformationMatrix();
    matrix = glm::scale(matrix, glm::vec3(window_size.x/2.0, window_size.y/2.0, 1.0));

    return glm::inverse(matrix);
}

Texture::Texture(Color color, IVector2 size) : size(size) {
    /*
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::vector<Color> image = std::vector<Color>(size.x*size.y, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_FLOAT, image.data());
    glGenerateMipmap(GL_TEXTURE_2D); */
}

void Texture::use(unsigned slot) {
    /*
    if (slot > 31) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID); */
}
#include "components.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/device.hpp"
#include "shader.hpp"

using namespace engine;

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

Mesh::Mesh(std::vector<Vector2> vertices, std::vector<unsigned> indices) : vertices(vertices), indices(indices) {
    vertexBuffer = new VertexBuffer(sizeof(glm::vec2));
    vertexBuffer->setVector(vertices.data(), vertices.size());

    indexBuffer = new IndexBuffer(sizeof(unsigned));
    indexBuffer->setVector(indices.data(), indices.size());
}

Mesh::Mesh(const Mesh& mesh) : vertices(mesh.vertices), indices(mesh.indices) {
    vertexBuffer = new VertexBuffer(sizeof(glm::vec2));
    vertexBuffer->setVector(vertices.data(), vertices.size());

    indexBuffer = new IndexBuffer(sizeof(unsigned));
    indexBuffer->setVector(indices.data(), indices.size());
}

Mesh::Mesh(Mesh&& mesh) : vertices(mesh.vertices), indices(mesh.indices), vertexBuffer(mesh.vertexBuffer), indexBuffer(mesh.indexBuffer) {
    mesh.vertexBuffer = nullptr;
    mesh.indexBuffer = nullptr;
}

Mesh::~Mesh()  {
    if (vertexBuffer) {
        delete vertexBuffer;
    }
    if (indexBuffer) {
        delete indexBuffer;
    }
}
#include "components.hpp"

using namespace engine;

void Mesh::bind() {
    if (VAO == 0 || VBO == 0 || EBO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vector2), verticies.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned), indicies.data(), GL_STATIC_DRAW);
    }
    glBindVertexArray(VAO);
}

void UVMesh::bind() {
    if (VAO == 0 || VBO == 0 || EBO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        std::vector<Vector2> combined_verticies = std::vector<Vector2>();

        for (unsigned v = 0; v < verticies.size(); v++) {
            combined_verticies.push_back(verticies[v]);
            combined_verticies.push_back(UVs[v]);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, combined_verticies.size() * sizeof(Vector2), combined_verticies.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned), indicies.data(), GL_STATIC_DRAW);
    }
    glBindVertexArray(VAO);
}

glm::mat4 Transform::getTransformationMatrix() {
    glm::mat4 matrix = glm::mat4(1);
    matrix = glm::rotate(matrix, (float)rotation, glm::vec3(0, 0, 1));
    matrix = glm::scale(matrix, glm::vec3(scale.x, scale.y, 1.0));
    matrix = glm::translate(matrix, glm::vec3(position.x, position.y, 0.0));
    return matrix;
}

glm::mat4 Camera::getProjectionMatrix(Transform& transform, IVector2 window_size) {
    int viewport[4];
    glm::mat4 matrix = transform.getTransformationMatrix();
    matrix = glm::scale(matrix, glm::vec3(window_size.x/2.0, window_size.y/2.0, 1.0));

    return glm::inverse(matrix);
}

Texture::Texture(Color color, IVector2 size) : size(size) {
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
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::use(unsigned slot) {
    if (slot > 31) {
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}
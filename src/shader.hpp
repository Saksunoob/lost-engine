#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine {
    class Shader
    {
    public:
        // the program ID
        unsigned int ID;

        // constructor reads and builds the shader
        Shader(const char* vertexPath, const char* fragmentPath);
        Shader(const char* dirPath);
        Shader(unsigned int ID) : ID(ID) {};
        // use/activate the shader
        void use();
        // utility uniform functions

        void setBVec1(const std::string& name, glm::bvec1 value) const;
        void setBVec2(const std::string& name, glm::bvec2 value) const;
        void setBVec3(const std::string& name, glm::bvec3 value) const;
        void setBVec4(const std::string& name, glm::bvec4 value) const;
        void setBool(const std::string& name, bool value) const;

        void setFVec1(const std::string& name, glm::fvec1 value) const;
        void setFVec2(const std::string& name, glm::fvec2 value) const;
        void setFVec3(const std::string& name, glm::fvec3 value) const;
        void setFVec4(const std::string& name, glm::fvec4 value) const;
        void setFloat(const std::string& name, float value) const;

        void setIVec1(const std::string& name, glm::ivec1 value) const;
        void setIVec2(const std::string& name, glm::ivec2 value) const;
        void setIVec3(const std::string& name, glm::ivec3 value) const;
        void setIVec4(const std::string& name, glm::ivec4 value) const;
        void setInt(const std::string& name, int value) const;

        void setUVec2(const std::string& name, glm::uvec2 value) const;
        void setUVec3(const std::string& name, glm::uvec3 value) const;
        void setUVec4(const std::string& name, glm::uvec4 value) const;
        void setUVec1(const std::string& name, glm::uvec1 value) const;
        void setUInt(const std::string& name, unsigned int value) const;

        void setMatrix2f(const std::string& name, glm::mat2 value) const;
        void setMatrix3f(const std::string& name, glm::mat3 value) const;
        void setMatrix4f(const std::string& name, glm::mat4 value) const;

    private:
        void checkCompileErrors(unsigned int shader, std::string type);
    };
}

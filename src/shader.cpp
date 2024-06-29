#include "shader.hpp"

using namespace engine;

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
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
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    // vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(const char* dirPath) {
    std::string vertexPath(dirPath);
    std::string fragmentPath(dirPath);

    vertexPath.append("/vertex.glsl");
    fragmentPath.append("/fragment.glsl");

    Shader shader(vertexPath.c_str(), fragmentPath.c_str());
    ID = shader.ID;
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use()
{
    glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------

// Boolean functions

void Shader::setBVec1(const std::string& name, glm::bvec1 value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value.x);
}
void Shader::setBVec2(const std::string& name, glm::bvec2 value) const {
    glUniform2i(glGetUniformLocation(ID, name.c_str()), (int)value.x, (int)value.y);
}
void Shader::setBVec3(const std::string& name, glm::bvec3 value) const {
    glUniform3i(glGetUniformLocation(ID, name.c_str()), (int)value.x, (int)value.y, (int)value.z);
}
void Shader::setBVec4(const std::string& name, glm::bvec4 value) const {
    glUniform4i(glGetUniformLocation(ID, name.c_str()), (int)value.x, (int)value.y, (int)value.z, (int)value.w);
}
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

// Float functions

void Shader::setFVec1(const std::string& name, glm::fvec1 value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value.x);
}
void Shader::setFVec2(const std::string& name, glm::fvec2 value) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}
void Shader::setFVec3(const std::string& name, glm::fvec3 value) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}
void Shader::setFVec4(const std::string& name, glm::fvec4 value) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

// Integer functions

void Shader::setIVec1(const std::string& name, glm::ivec1 value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value.x);
}
void Shader::setIVec2(const std::string& name, glm::ivec2 value) const {
    glUniform2i(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}
void Shader::setIVec3(const std::string& name, glm::ivec3 value) const {
    glUniform3i(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}
void Shader::setIVec4(const std::string& name, glm::ivec4 value) const {
    glUniform4i(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

// Unsigned Integer functions

void Shader::setUVec1(const std::string& name, glm::uvec1 value) const {
    glUniform1ui(glGetUniformLocation(ID, name.c_str()), value.x);
}
void Shader::setUVec2(const std::string& name, glm::uvec2 value) const {
    glUniform2ui(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}
void Shader::setUVec3(const std::string& name, glm::uvec3 value) const {
    glUniform3ui(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}
void Shader::setUVec4(const std::string& name, glm::uvec4 value) const {
    glUniform4ui(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}
void Shader::setUInt(const std::string& name, unsigned int value) const {
    glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
}

// Matricies

void Shader::setMatrix2f(const std::string& name, glm::mat2 value) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setMatrix3f(const std::string& name, glm::mat3 value) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setMatrix4f(const std::string& name, glm::mat4 value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}


void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
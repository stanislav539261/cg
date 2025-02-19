#include <fstream>
#include <iostream>

#include "shader.hpp"

ShaderProgram::ShaderProgram() {
    m_Handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_Handle);
}

bool ShaderProgram::Link(GLenum stage, const std::filesystem::path &filename) const {
    auto shader = glCreateShader(stage);
    auto file = std::ifstream(filename);

    if (file.is_open()) {
        auto src = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        auto srcCStr = src.c_str();

        glShaderSource(shader, 1, &srcCStr, nullptr);
        glCompileShader(shader);
    } else {
        std::cout << "Can't open shader: " << filename;
        return false;
    }

    glAttachShader(m_Handle, shader);
    glLinkProgram(m_Handle);
    glDeleteShader(shader);
    return true;
}

void ShaderProgram::SetUniform(GLuint location, GLint value) const {
    glProgramUniform1i(m_Handle, location, value);
}

void ShaderProgram::SetUniform(GLuint location, GLuint value) const {
    glProgramUniform1ui(m_Handle, location, value);
}

void ShaderProgram::SetUniform(GLuint location, GLfloat value) const {
    glProgramUniform1f(m_Handle, location, value);
}

void ShaderProgram::SetUniform(GLuint location, const glm::vec2 &value) const {
    glProgramUniform2fv(m_Handle, location, 1, reinterpret_cast<const float *>(&value));
}

void ShaderProgram::SetUniform(GLuint location, const glm::vec3 &value) const {
    glProgramUniform3fv(m_Handle, location, 1, reinterpret_cast<const float *>(&value));
}

void ShaderProgram::SetUniform(GLuint location, const glm::vec4 &value) const {
    glProgramUniform4fv(m_Handle, location, 1, reinterpret_cast<const float *>(&value));
}

void ShaderProgram::Use() const {
    glUseProgram(m_Handle);
}
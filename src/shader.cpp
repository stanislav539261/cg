#include <fstream>
#include <iostream>

#include "shader.hpp"

ShaderProgram::ShaderProgram() {
    m_Handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_Handle);
}

bool ShaderProgram::Link(GLenum stage, const std::filesystem::path &filename) {
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

void ShaderProgram::Use() {
    glUseProgram(m_Handle);
}
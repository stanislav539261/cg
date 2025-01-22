#ifndef SHADER_HPP
#define SHADER_HPP

#include <filesystem>

#include <GL/glew.h> 

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool    Link(GLenum, const std::filesystem::path &);
    void    Use();

    GLuint  m_Handle;
};

#endif /* SHADER_HPP */


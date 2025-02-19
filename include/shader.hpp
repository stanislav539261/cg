#ifndef SHADER_HPP
#define SHADER_HPP

#include <filesystem>

#include <GL/glew.h> 
#include <glm/glm.hpp>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool    Link(GLenum, const std::filesystem::path &) const;
    void    SetUniform(GLuint, GLint) const;
    void    SetUniform(GLuint, GLuint) const;
    void    SetUniform(GLuint, GLfloat) const;
    void    SetUniform(GLuint, const glm::vec2 &) const;
    void    SetUniform(GLuint, const glm::vec3 &) const;
    void    SetUniform(GLuint, const glm::vec4 &) const;
    void    Use() const;

    GLuint  m_Handle;
};

#endif /* SHADER_HPP */


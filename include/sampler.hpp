#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include <GL/glew.h> 
#include <glm/glm.hpp>

class Sampler {
public:
    Sampler();
    ~Sampler();

    void    Bind(GLuint) const;
    void    SetParameter(GLenum, GLfloat) const;
    void    SetParameter(GLenum, GLuint) const;
    void    SetParameter(GLenum, const glm::vec2 &) const;
    void    SetParameter(GLenum, const glm::vec3 &) const;
    void    SetParameter(GLenum, const glm::vec4 &) const;

    GLuint  m_Handle;
};

#endif /* SAMPLER_HPP */
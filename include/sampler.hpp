#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include <GL/glew.h> 
#include <glm/glm.hpp>

class Sampler {
public:
    Sampler();
    ~Sampler();

    void    SetParameter(GLenum, GLfloat);
    void    SetParameter(GLenum, GLuint);
    void    SetParameter(GLenum, const glm::vec2 &);
    void    SetParameter(GLenum, const glm::vec3 &);
    void    SetParameter(GLenum, const glm::vec4 &);

    GLuint  m_Handle;
};

#endif /* SAMPLER_HPP */
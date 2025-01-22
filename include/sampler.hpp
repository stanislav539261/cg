#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include <GL/glew.h> 

class Sampler {
public:
    Sampler();
    ~Sampler();

    void    SetParameter(GLenum, GLfloat);
    void    SetParameter(GLenum, GLuint);

    GLuint  m_Handle;
};

#endif /* SAMPLER_HPP */
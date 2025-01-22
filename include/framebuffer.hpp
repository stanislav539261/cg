#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <memory>

#include <GL/glew.h> 

#include "texture.hpp"

class DefaultFramebuffer {
public:
    DefaultFramebuffer();
    ~DefaultFramebuffer();

    void    Bind();
    void    ClearColor(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
    void    ClearDepth(GLuint, GLfloat);
};

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    void    Bind();
    void    ClearColor(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
    void    ClearDepth(GLuint, GLfloat);
    void    SetAttachment(GLenum, const std::shared_ptr<Texture2D> &);

    GLuint  m_Handle;
};

#endif /* FRAMEBUFFER_HPP */
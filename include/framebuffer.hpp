#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/glew.h> 

#include "texture.hpp"

class DefaultFramebuffer {
public:
    static void     Bind();
    static void     ClearColor(const glm::vec4 &);
    static void     ClearDepth(GLfloat);
};

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    void    Bind() const;
    void    ClearColor(GLuint, const glm::vec4 &) const;
    void    ClearDepth(GLuint, GLfloat) const;
    void    SetAttachment(GLenum, const Texture *) const;
    void    SetAttachment(GLenum, const TextureView *) const;

    GLuint  m_Handle;
};

#endif /* FRAMEBUFFER_HPP */
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
    void    SetAttachment(GLenum, const std::shared_ptr<Texture2DArray> &);
    void    SetAttachment(GLenum, const std::shared_ptr<TextureCube> &);
    void    SetAttachment(GLenum, const std::shared_ptr<TextureCubeArray> &);
    void    SetAttachment(GLenum, const std::shared_ptr<TextureView2D> &);
    void    SetAttachment(GLenum, const std::shared_ptr<TextureViewCube> &);

    GLuint  m_Handle;
};

#endif /* FRAMEBUFFER_HPP */
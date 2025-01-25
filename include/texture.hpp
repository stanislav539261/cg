#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vector>

#include <GL/glew.h> 

#include "sampler.hpp"
#include "image.hpp"

class Texture {
public:
    void    Bind(GLuint);
    void    Bind(GLuint, const std::shared_ptr<const Sampler> &);
    void    GenerateMipMaps();
    void    SetParameter(GLenum, GLfloat);
    void    SetParameter(GLenum, GLuint);
    void    SetParameter(GLenum, const glm::vec2 &);
    void    SetParameter(GLenum, const glm::vec3 &);
    void    SetParameter(GLenum, const glm::vec4 &);

    GLuint  m_Handle;
    GLuint  m_MipLevel;
    GLuint  m_Format;
    GLenum  m_Target;
};

class Texture2D : public Texture {
public:
    Texture2D(GLuint, GLuint, GLuint, GLuint);
    Texture2D(const Image &);
    ~Texture2D();

    GLuint  m_Width;
    GLuint  m_Height;
};

class Texture2DArray : public Texture {
public:
    Texture2DArray(GLuint, GLuint, GLuint, GLuint, GLuint);
    Texture2DArray(const std::vector<std::shared_ptr<Image>> &);
    ~Texture2DArray();
    
    GLuint  m_Width;
    GLuint  m_Height;
    GLuint  m_Depth;
};

class TextureView {
public:
    void    Bind(GLuint);
    void    Bind(GLuint, const std::shared_ptr<const Sampler> &);

    GLuint                          m_Handle;
    GLenum                          m_Target;
    std::shared_ptr<const Texture>  m_Texture;
};

class TextureView2D : public TextureView {
public:
    TextureView2D(std::shared_ptr<const Texture>, GLuint, GLuint);
    ~TextureView2D();
};

#endif /* TEXTURE_HPP */
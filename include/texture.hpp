#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vector>

#include <GL/glew.h> 

#include "sampler.hpp"
#include "image.hpp"

class Texture2D {
public:
    Texture2D(GLuint, GLuint, GLuint, GLuint);
    Texture2D(const Image &);
    ~Texture2D();

    void    Bind(GLuint);
    void    Bind(GLuint, const Sampler &);

    GLuint  m_Handle;
    GLuint  m_Width;
    GLuint  m_Height;
    GLuint  m_MipLevel;
    GLuint  m_Format;
};

class Texture2DArray {
public:
    Texture2DArray(GLuint, GLuint, GLuint, GLuint, GLuint);
    Texture2DArray(const std::vector<std::shared_ptr<Image>> &);
    ~Texture2DArray();

    void    Bind(GLuint);
    void    Bind(GLuint, const Sampler &);
    
    GLuint  m_Handle;
    GLuint  m_Width;
    GLuint  m_Height;
    GLuint  m_Depth;
    GLuint  m_MipLevel;
    GLuint  m_Format;
};

#endif /* TEXTURE_HPP */
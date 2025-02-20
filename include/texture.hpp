#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h> 

#include "sampler.hpp"
#include "image.hpp"

class Texture {
public:
    virtual ~Texture() = default;

    void            Bind(GLuint) const;
    void            Bind(GLuint, const Sampler *) const;
    void            GenerateMipMaps() const;
    virtual bool    Is2D() const = 0;
    virtual bool    Is2DArray() const = 0;
    virtual bool    IsCube() const = 0;
    virtual bool    IsCubeArray() const = 0;
    void            SetParameter(GLenum, GLfloat) const;
    void            SetParameter(GLenum, GLuint) const;
    void            SetParameter(GLenum, const glm::vec2 &) const;
    void            SetParameter(GLenum, const glm::vec3 &) const;
    void            SetParameter(GLenum, const glm::vec4 &) const;
    virtual GLenum  Target() const { return GL_NONE; }

    glm::uvec3      m_Extent;
    GLenum          m_Format;
    GLuint          m_Handle;
    GLuint          m_MipLevel;

protected:
    Texture() = default;
};

class Texture2D : public Texture {
public:
    Texture2D(const glm::uvec2 &, GLuint, GLenum);
    ~Texture2D();

    void    Copy(const Texture *, const glm::uvec2 &, GLuint, const glm::uvec3 &, GLuint) const;
    bool    Is2D() const override { return true; }
    bool    Is2DArray() const override { return false; }
    bool    IsCube() const override { return false; }
    bool    IsCubeArray() const override { return false; }
    GLenum  Target() const override { return GL_TEXTURE_2D; }
    void    Upload(const Image *, const glm::uvec2 &, GLuint) const;
};

class Texture2DArray : public Texture {
public:
    Texture2DArray(const glm::uvec3 &, GLuint, GLenum);
    ~Texture2DArray();

    void    Copy(const Texture *, const glm::uvec3 &, GLuint, const glm::uvec3 &, GLuint) const;
    bool    Is2D() const override { return false; }
    bool    Is2DArray() const override { return true; }
    bool    IsCube() const override { return false; }
    bool    IsCubeArray() const override { return false; }
    GLenum  Target() const override { return GL_TEXTURE_2D_ARRAY; }
    void    Upload(const Image *, const glm::uvec3 &, GLuint) const;
};

class TextureCube : public Texture {
public:
    TextureCube(const glm::uvec2 &, GLuint, GLenum);
    ~TextureCube();

    void    Copy(const Texture *, const glm::uvec3 &, GLuint, const glm::uvec3 &, GLuint) const;
    bool    Is2D() const override { return false; }
    bool    Is2DArray() const override { return false; }
    bool    IsCube() const override { return true; }
    bool    IsCubeArray() const override { return false; }
    GLenum  Target() const override { return GL_TEXTURE_CUBE_MAP; }
    void    Upload(const Image *, const glm::uvec3 &, GLuint) const;
};

class TextureCubeArray : public Texture {
public:
    TextureCubeArray(const glm::uvec3 &, GLuint, GLenum);
    ~TextureCubeArray();

    void    Copy(const Texture *, const glm::uvec3 &, GLuint, const glm::uvec3 &, GLuint) const;
    bool    Is2D() const override { return false; }
    bool    Is2DArray() const override { return false; }
    bool    IsCube() const override { return false; }
    bool    IsCubeArray() const override { return true; }
    GLenum  Target() const override { return GL_TEXTURE_CUBE_MAP_ARRAY; }
    void    Upload(const Image *, const glm::uvec3 &, GLuint) const;
};

class TextureView {
public:
    virtual ~TextureView() = default;

    void                Bind(GLuint) const;
    void                Bind(GLuint, const Sampler *) const;
    virtual bool        Is2D() const = 0;
    virtual bool        Is2DArray() const = 0;
    virtual bool        IsCube() const = 0;
    virtual bool        IsCubeArray() const = 0;
    virtual GLenum      Target() const { return GL_NONE; }

    GLuint              m_Handle;
    const Texture *     m_Texture;

protected:
    TextureView() = default;
};

class TextureView2D : public TextureView {
public:
    TextureView2D(const Texture *, GLuint, GLuint, GLuint);
    ~TextureView2D();

    bool    Is2D() const override { return true; }
    bool    Is2DArray() const override { return false; }
    bool    IsCube() const override { return false; }
    bool    IsCubeArray() const override { return false; }
    GLenum  Target() const override { return GL_TEXTURE_2D; }
};

class TextureViewCube : public TextureView {
public:
    TextureViewCube(const Texture *, GLuint, GLuint, GLuint);
    ~TextureViewCube();

    bool    Is2D() const override { return false; }
    bool    Is2DArray() const override { return false; }
    bool    IsCube() const override { return true; }
    bool    IsCubeArray() const override { return false; }
    GLenum  Target() const override { return GL_TEXTURE_CUBE_MAP; }
};

#endif /* TEXTURE_HPP */
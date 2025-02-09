#include <iostream>

#include "texture.hpp"

void Texture::Bind(GLuint binding) {
    glBindTextureUnit(binding, m_Handle);
}

void Texture::Bind(GLuint binding, const std::shared_ptr<const Sampler> &sampler) {
    Bind(binding);

    glBindSampler(binding, sampler->m_Handle);
}

void Texture::GenerateMipMaps() {
    glGenerateTextureMipmap(m_Handle);
}

void Texture::SetParameter(GLenum pname, GLfloat param) {
    glTextureParameterf(m_Handle, pname, param);
}

void Texture::SetParameter(GLenum pname, GLuint param) {
    glTextureParameteri(m_Handle, pname, param);
}

void Texture::SetParameter(GLenum pname, const glm::vec2 &param) {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Texture::SetParameter(GLenum pname, const glm::vec3 &param) {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Texture::SetParameter(GLenum pname, const glm::vec4 &param) {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

Texture2D::Texture2D(GLuint width, GLuint height, GLuint mipLevel, GLuint format) {
    m_Target = GL_TEXTURE_2D;

    glCreateTextures(m_Target, 1, &m_Handle);
    glTextureStorage2D(m_Handle, mipLevel, format, width, height);

    m_Format = format;
    m_Height = height;
    m_MipLevel = mipLevel;
    m_Width = width;
}

Texture2D::Texture2D(const Image &texture) {
    auto format = GL_NONE;
    auto internalFormat = GL_NONE;
    auto mipLevel = texture.MipLevel();

    switch (texture.m_Channels) {
        case 1:
            format = GL_R;
            internalFormat = GL_R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            break;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_Handle);
    glTextureStorage2D(m_Handle, mipLevel, internalFormat, texture.m_Width, texture.m_Height);
    glTextureSubImage2D(m_Handle, 0, 0, 0, texture.m_Width, texture.m_Height, format, GL_UNSIGNED_BYTE, texture.m_Data);
    glGenerateTextureMipmap(m_Handle);

    m_Format = format;
    m_Height = texture.m_Height;
    m_MipLevel = mipLevel;
    m_Width = texture.m_Width;
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_Handle);
}

Texture2DArray::Texture2DArray(GLuint width, GLuint height, GLuint depth, GLuint mipLevel, GLuint format) {
    m_Target = GL_TEXTURE_2D_ARRAY;

    glCreateTextures(m_Target, 1, &m_Handle);
    glTextureStorage3D(m_Handle, mipLevel, format, width, height, depth);

    m_Format = format;
    m_Depth = depth;
    m_Height = height;
    m_MipLevel = mipLevel;
    m_Width = width;
}

Texture2DArray::Texture2DArray(const std::vector<std::shared_ptr<Image>> &textures) {
    if (textures.size() > 0) {
        auto format = GL_NONE;
        auto internalFormat = GL_NONE;
        auto mipLevel = textures[0]->MipLevel();

        switch (textures[0]->m_Channels) {
            case 1:
                format = GL_RED;
                internalFormat = GL_R8;
                break;
            case 2:
                format = GL_RG;
                internalFormat = GL_RG8;
                break;
            case 3:
                format = GL_RGB;
                internalFormat = GL_RGB8;
                break;
            case 4:
                format = GL_RGBA;
                internalFormat = GL_RGBA8;
                break;
            default:
                break;
        }

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_Handle);
        glTextureStorage3D(m_Handle, mipLevel, internalFormat, textures[0]->m_Width, textures[0]->m_Height, textures.size());
        
        for (auto i = 0u; i < textures.size(); i++) {
            if (textures[i]->m_Width != textures[0]->m_Width) {
                std::cout << "Width of texture " << i + 1 << " isn't equal to first" << std::endl;
                continue;
            }
            if (textures[i]->m_Height != textures[0]->m_Height) {
                std::cout << "Height of texture " << i + 1 << " isn't equal to first" << std::endl;
                continue;
            }
            if (textures[i]->m_Channels != textures[0]->m_Channels) {
                std::cout << "Channels of texture " << i + 1 << " isn't equal to first" << std::endl;
                continue;
            }

            glTextureSubImage3D(m_Handle, 0, 0, 0, i, textures[0]->m_Width, textures[0]->m_Height, 1, format, GL_UNSIGNED_BYTE, textures[i]->m_Data);
        }

        glGenerateTextureMipmap(m_Handle);

        m_Format = format;
        m_Height = textures[0]->m_Height;
        m_MipLevel = mipLevel;
        m_Width = textures[0]->m_Width;
    } else {
        std::cout << "Can't create Texture2DArray: num textures is zero" << std::endl;
    }
}

Texture2DArray::~Texture2DArray() {
    glDeleteTextures(1, &m_Handle);
}

TextureCube::TextureCube(GLuint width, GLuint height, GLuint mipLevel, GLuint format) {
    m_Target = GL_TEXTURE_CUBE_MAP;

    glCreateTextures(m_Target, 1, &m_Handle);
    glTextureStorage2D(m_Handle, mipLevel, format, width, height);

    m_Format = format;
    m_Height = height;
    m_MipLevel = mipLevel;
    m_Width = width;
}

TextureCube::~TextureCube() {
    glDeleteTextures(1, &m_Handle);
}

TextureCubeArray::TextureCubeArray(GLuint width, GLuint height, GLuint depth, GLuint mipLevel, GLuint format) {
    m_Target = GL_TEXTURE_CUBE_MAP_ARRAY;

    glCreateTextures(m_Target, 1, &m_Handle);
    glTextureStorage3D(m_Handle, mipLevel, format, width, height, depth);

    m_Format = format;
    m_Height = height;
    m_MipLevel = mipLevel;
    m_Width = width;
}

TextureCubeArray::~TextureCubeArray() {
    glDeleteTextures(1, &m_Handle);
}

void TextureView::Bind(GLuint binding) {
    glBindTextureUnit(binding, m_Handle);
}

void TextureView::Bind(GLuint binding, const std::shared_ptr<const Sampler> &sampler) {
    Bind(binding);

    glBindSampler(binding, sampler->m_Handle);
}

TextureView2D::TextureView2D(std::shared_ptr<const Texture> texture, GLuint minLevel, GLuint numLevels, GLuint minLayer) {
    glGenTextures(1, &m_Handle);
    glTextureView(m_Handle, GL_TEXTURE_2D, texture->m_Handle, texture->m_Format, minLevel, numLevels, minLayer, 1);
}

TextureView2D::~TextureView2D() {
    glDeleteTextures(1, &m_Handle);
}

TextureViewCube::TextureViewCube(std::shared_ptr<const Texture> texture, GLuint minLevel, GLuint numLevels, GLuint minLayer) {
    glGenTextures(1, &m_Handle);
    glTextureView(m_Handle, GL_TEXTURE_CUBE_MAP, texture->m_Handle, texture->m_Format, minLevel, numLevels, minLayer, 6);
}

TextureViewCube::~TextureViewCube() {
    glDeleteTextures(1, &m_Handle);
}
#include "texture.hpp"

static std::tuple<GLuint, GLuint, GLuint> FindImageFormat(const Image *image) {
    auto format = GL_NONE;
    auto internalFormat = GL_NONE;
    auto type = GL_UNSIGNED_BYTE;

    switch (image->m_Channels) {
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

    return std::make_tuple(format, internalFormat, type);
}

void Texture::Bind(GLuint binding) const {
    glBindTextureUnit(binding, m_Handle);
}

void Texture::Bind(GLuint binding, const Sampler *sampler) const {
    Bind(binding);

    sampler->Bind(binding);
}

void Texture::GenerateMipMaps() const {
    glGenerateTextureMipmap(m_Handle);
}

void Texture::SetParameter(GLenum pname, GLfloat param) const {
    glTextureParameterf(m_Handle, pname, param);
}

void Texture::SetParameter(GLenum pname, GLuint param) const {
    glTextureParameteri(m_Handle, pname, param);
}

void Texture::SetParameter(GLenum pname, const glm::vec2 &param) const {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Texture::SetParameter(GLenum pname, const glm::vec3 &param) const {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Texture::SetParameter(GLenum pname, const glm::vec4 &param) const {
    glTextureParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

Texture2D::Texture2D(const glm::uvec2 &extent, GLuint mipLevel, GLenum format) : Texture() {
    glCreateTextures(Target(), 1, &m_Handle);
    glTextureStorage2D(m_Handle, mipLevel, format, extent.x, extent.y);

    m_Extent = glm::uvec3(extent, 1u);
    m_Format = format;
    m_MipLevel = mipLevel;
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_Handle);
}

void Texture2D::Copy(const Texture *dst, const glm::uvec2 &srcOffset, GLuint srcLevel, const glm::uvec3 &dstOffset, GLuint dstLevel) const {
    glCopyImageSubData(
        m_Handle, 
        Target(), 
        srcLevel, 
        srcOffset.x, 
        srcOffset.y, 
        1, 
        dst->m_Handle, 
        dst->Target(), 
        dstLevel, 
        dstOffset.x, 
        dstOffset.y,
        dstOffset.z,
        m_Extent.x, 
        m_Extent.y, 
        m_Extent.z
    );
}

void Texture2D::Upload(const Image *upload, const glm::uvec2 &offset, GLuint level) const {
    auto [format, internalFormat, type] = FindImageFormat(upload);
    auto mipLevel = upload->MipLevel();

    assert(m_Extent.x == upload->m_Width);
    assert(m_Extent.y == upload->m_Height);

    glTextureSubImage2D(m_Handle, level, offset.x, offset.y, upload->m_Width, upload->m_Height, format, type, upload->m_Data);
}

Texture2DArray::Texture2DArray(const glm::uvec3 &extent, GLuint mipLevel, GLenum format) : Texture() {
    glCreateTextures(Target(), 1, &m_Handle);
    glTextureStorage3D(m_Handle, mipLevel, format, extent.x, extent.y, extent.z);

    m_Extent = extent;
    m_Format = format;
    m_MipLevel = mipLevel;
}

Texture2DArray::~Texture2DArray() {
    glDeleteTextures(1, &m_Handle);
}

void Texture2DArray::Copy(const Texture *dst, const glm::uvec3 &srcOffset, GLuint srcLevel, const glm::uvec3 &dstOffset, GLuint dstLevel) const {
    glCopyImageSubData(
        m_Handle, 
        Target(), 
        srcLevel, 
        srcOffset.x, 
        srcOffset.y, 
        srcOffset.z, 
        dst->m_Handle, 
        dst->Target(), 
        dstLevel, 
        dstOffset.x, 
        dstOffset.y,
        dstOffset.z,
        m_Extent.x, 
        m_Extent.y, 
        m_Extent.z
    );
}

void Texture2DArray::Upload(const Image *upload, const glm::uvec3 &offset, GLuint level) const {
    auto [format, internalFormat, type] = FindImageFormat(upload);
    auto mipLevel = upload->MipLevel();

    assert(m_Extent.x == upload->m_Width);
    assert(m_Extent.y == upload->m_Height);

    glTextureSubImage3D(m_Handle, level, offset.x, offset.y, offset.z, upload->m_Width, upload->m_Height, 1, format, type, upload->m_Data);
}

TextureCube::TextureCube(const glm::uvec2 &extent, GLuint mipLevel, GLenum format) : Texture() {
    glCreateTextures(Target(), 1, &m_Handle);
    glTextureStorage2D(m_Handle, mipLevel, format, extent.x, extent.y);

    m_Extent = glm::uvec3(extent, 6u);
    m_Format = format;
    m_MipLevel = mipLevel;
}

TextureCube::~TextureCube() {
    glDeleteTextures(1, &m_Handle);
}

void TextureCube::Copy(const Texture *dst, const glm::uvec3 &srcOffset, GLuint srcLevel, const glm::uvec3 &dstOffset, GLuint dstLevel) const {
    glCopyImageSubData(
        m_Handle, 
        Target(), 
        srcLevel, 
        srcOffset.x, 
        srcOffset.y, 
        srcOffset.z, 
        dst->m_Handle, 
        dst->Target(), 
        dstLevel, 
        dstOffset.x, 
        dstOffset.y,
        dstOffset.z,
        m_Extent.x, 
        m_Extent.y, 
        m_Extent.z
    );
}

void TextureCube::Upload(const Image *upload, const glm::uvec3 &offset, GLuint level) const {
    auto [format, internalFormat, type] = FindImageFormat(upload);
    auto mipLevel = upload->MipLevel();

    assert(m_Extent.x == upload->m_Width);
    assert(m_Extent.y == upload->m_Height);

    glTextureSubImage3D(m_Handle, level, offset.x, offset.y, offset.z, upload->m_Width, upload->m_Height, 1, format, type, upload->m_Data);
}

TextureCubeArray::TextureCubeArray(const glm::uvec3 &extent, GLuint mipLevel, GLenum format) : Texture() {
    glCreateTextures(Target(), 1, &m_Handle);
    glTextureStorage3D(m_Handle, mipLevel, format, extent.x, extent.y, extent.z);

    m_Extent = extent;
    m_Format = format;
    m_MipLevel = mipLevel;
}

TextureCubeArray::~TextureCubeArray() {
    glDeleteTextures(1, &m_Handle);
}

void TextureCubeArray::Copy(const Texture *dst, const glm::uvec3 &srcOffset, GLuint srcLevel, const glm::uvec3 &dstOffset, GLuint dstLevel) const {
    glCopyImageSubData(
        m_Handle, 
        Target(), 
        srcLevel, 
        srcOffset.x, 
        srcOffset.y, 
        srcOffset.z, 
        dst->m_Handle, 
        dst->Target(), 
        dstLevel, 
        dstOffset.x, 
        dstOffset.y,
        dstOffset.z,
        m_Extent.x, 
        m_Extent.y, 
        m_Extent.z
    );
}

void TextureCubeArray::Upload(const Image *upload, const glm::uvec3 &offset, GLuint level) const {
    auto [format, internalFormat, type] = FindImageFormat(upload);
    auto mipLevel = upload->MipLevel();

    assert(m_Extent.x == upload->m_Width);
    assert(m_Extent.y == upload->m_Height);

    glTextureSubImage3D(m_Handle, level, offset.x, offset.y, offset.z, upload->m_Width, upload->m_Height, 1, format, type, upload->m_Data);
}

void TextureView::Bind(GLuint binding) const {
    glBindTextureUnit(binding, m_Handle);
}

void TextureView::Bind(GLuint binding, const Sampler *sampler) const {
    Bind(binding);

    sampler->Bind(binding);
}

TextureView2D::TextureView2D(const Texture *texture, GLuint minLevel, GLuint numLevels, GLuint minLayer) : TextureView() {
    glGenTextures(1, &m_Handle);
    glTextureView(m_Handle, Target(), texture->m_Handle, texture->m_Format, minLevel, numLevels, minLayer, 1);

    m_Texture = texture;
}

TextureView2D::~TextureView2D() {
    glDeleteTextures(1, &m_Handle);
}

TextureViewCube::TextureViewCube(const Texture *texture, GLuint minLevel, GLuint numLevels, GLuint minLayer) : TextureView() {
    glGenTextures(1, &m_Handle);
    glTextureView(m_Handle, Target(), texture->m_Handle, texture->m_Format, minLevel, numLevels, minLayer, 6);

    m_Texture = texture;
}

TextureViewCube::~TextureViewCube() {
    glDeleteTextures(1, &m_Handle);
}
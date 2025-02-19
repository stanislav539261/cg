#include "framebuffer.hpp"

void DefaultFramebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DefaultFramebuffer::ClearColor(const glm::vec4 &value) {
    glClearNamedFramebufferfv(0, GL_COLOR, 0, reinterpret_cast<const GLfloat *>(&value));
}

void DefaultFramebuffer::ClearDepth(GLfloat value) {
    glClearNamedFramebufferfv(0, GL_DEPTH, 0, &value);
}

Framebuffer::Framebuffer() {
    glCreateFramebuffers(1, &m_Handle);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_Handle);
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
}

void Framebuffer::ClearColor(GLuint attachment, const glm::vec4 &value) const {
    glClearNamedFramebufferfv(m_Handle, GL_COLOR, attachment, reinterpret_cast<const GLfloat *>(&value));
}

void Framebuffer::ClearDepth(GLuint attachment, GLfloat value) const {
    glClearNamedFramebufferfv(m_Handle, GL_DEPTH, attachment, &value);
}

void Framebuffer::SetAttachment(GLenum attachment, const Texture *texture) const {
    glNamedFramebufferTexture(m_Handle, attachment, texture->m_Handle, 0);
}

void Framebuffer::SetAttachment(GLenum attachment, const TextureView *texture) const {
    glNamedFramebufferTexture(m_Handle, attachment, texture->m_Handle, 0);
}
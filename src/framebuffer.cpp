#include "framebuffer.hpp"

DefaultFramebuffer::DefaultFramebuffer() {

}

DefaultFramebuffer::~DefaultFramebuffer() {

}

void DefaultFramebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DefaultFramebuffer::ClearColor(GLuint layer, GLfloat r, GLfloat b, GLfloat g, GLfloat a) {
    auto values = { r, g, b, a };

    glClearNamedFramebufferfv(0, GL_COLOR, 0, values.begin());
}

void DefaultFramebuffer::ClearDepth(GLuint layer, GLfloat d) {
    glClearNamedFramebufferfv(0, GL_DEPTH, 0, &d);
}

Framebuffer::Framebuffer() {
    glCreateFramebuffers(1, &m_Handle);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_Handle);
}

void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
}

void Framebuffer::ClearColor(GLuint layer, GLfloat r, GLfloat b, GLfloat g, GLfloat a) {
    auto values = { r, g, b, a };

    glClearNamedFramebufferfv(m_Handle, GL_COLOR, layer, values.begin());
}

void Framebuffer::ClearDepth(GLuint layer, GLfloat d) {
    glClearNamedFramebufferfv(m_Handle, GL_DEPTH, layer, &d);
}

void Framebuffer::SetAttachment(GLenum attachment, const std::shared_ptr<Texture2D> &texture) {
    glNamedFramebufferTexture(m_Handle, attachment, texture->m_Handle, 0);
}
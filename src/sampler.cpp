#include "sampler.hpp" 

Sampler::Sampler() {
    glCreateSamplers(1, &m_Handle);
}

Sampler::~Sampler() {
    glDeleteSamplers(1, &m_Handle);
}

void Sampler::SetParameter(GLenum pname, GLfloat param) {
    glSamplerParameterf(m_Handle, pname, param);
}

void Sampler::SetParameter(GLenum pname, GLuint param) {
    glSamplerParameteri(m_Handle, pname, param);
}

void Sampler::SetParameter(GLenum pname, const glm::vec2 &param) {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Sampler::SetParameter(GLenum pname, const glm::vec3 &param) {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Sampler::SetParameter(GLenum pname, const glm::vec4 &param) {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}
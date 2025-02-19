#include "sampler.hpp" 

Sampler::Sampler() {
    glCreateSamplers(1, &m_Handle);
}

Sampler::~Sampler() {
    glDeleteSamplers(1, &m_Handle);
}

void Sampler::Bind(GLuint binding) const {
    glBindSampler(binding, m_Handle);
}

void Sampler::SetParameter(GLenum pname, GLfloat param) const {
    glSamplerParameterf(m_Handle, pname, param);
}

void Sampler::SetParameter(GLenum pname, GLuint param) const {
    glSamplerParameteri(m_Handle, pname, param);
}

void Sampler::SetParameter(GLenum pname, const glm::vec2 &param) const {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Sampler::SetParameter(GLenum pname, const glm::vec3 &param) const {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}

void Sampler::SetParameter(GLenum pname, const glm::vec4 &param) const {
    glSamplerParameterfv(m_Handle, pname, reinterpret_cast<const GLfloat *>(&param));
}
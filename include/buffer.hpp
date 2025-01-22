#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>

#include <GL/glew.h> 

template<typename T> 
class Buffer {
public:
    Buffer() : Buffer(1) {};
    Buffer(GLsizei count);
    ~Buffer();

    void    Bind(GLuint);
    void    SetData(const T &, GLsizei);
    void    SetData(const std::vector<T> &, GLsizei);

    GLuint  m_Handle;
    GLsizei m_Count;
};

template<typename T>
inline Buffer<T>::Buffer(GLsizei count) {
    glCreateBuffers(1, &m_Handle);
    glNamedBufferStorage(m_Handle, count * sizeof(T), nullptr, GL_DYNAMIC_STORAGE_BIT);

    m_Count = count;
}

template<typename T>
inline Buffer<T>::~Buffer() {
    glDeleteBuffers(1, &m_Handle);
}

template<typename T> 
inline void Buffer<T>::Bind(GLuint binding) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_Handle);
}

template<typename T> 
inline void Buffer<T>::SetData(const T &data, GLsizei first) {
    glNamedBufferSubData(m_Handle, static_cast<size_t>(first) * sizeof(T), sizeof(T), &data);
}

template<typename T> 
inline void Buffer<T>::SetData(const std::vector<T> &data, GLsizei first) {
    glNamedBufferSubData(m_Handle, static_cast<size_t>(first) * sizeof(T), data.size() * sizeof(T), data.data());
}

struct DrawIndirectCommand {
    GLuint m_NumVertices;
    GLuint m_NumInstances;
    GLuint m_FirstVertex;
    GLuint m_FirstInstance;
};

class DrawIndirectBuffer : public Buffer<DrawIndirectCommand> {
public:
    DrawIndirectBuffer() : Buffer<DrawIndirectCommand>(1) {};
    DrawIndirectBuffer(GLsizei count) : Buffer<DrawIndirectCommand>(count) {};

    void    Bind();
    void    Draw(GLenum, GLsizei, GLsizei);
};

#endif /* BUFFER_HPP */
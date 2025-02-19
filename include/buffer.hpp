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

    void    BindStorage(GLuint) const;
    void    Copy(const Buffer<T> *, GLintptr, GLintptr, GLsizeiptr) const;
    void    Upload(const T &, GLsizei) const;
    void    Upload(const std::vector<T> &, GLsizei) const;

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
inline void Buffer<T>::BindStorage(GLuint binding) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_Handle);
}

template<typename T> 
inline void Buffer<T>::Copy(const Buffer<T> *dst, GLintptr srcFirst, GLintptr dstFirst, GLsizeiptr count) const {
    glCopyNamedBufferSubData(m_Handle, dst->m_Handle, srcFirst * sizeof(T), dstFirst * sizeof(T), count * sizeof(T));
}

template<typename T> 
inline void Buffer<T>::Upload(const T &data, GLsizei first) const {
    glNamedBufferSubData(m_Handle, static_cast<size_t>(first) * sizeof(T), sizeof(T), &data);
}

template<typename T> 
inline void Buffer<T>::Upload(const std::vector<T> &data, GLsizei first) const {
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

    void    BindIndirect() const;
};

#endif /* BUFFER_HPP */
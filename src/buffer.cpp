#include "buffer.hpp"

void DrawIndirectBuffer::Bind() {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
}

void DrawIndirectBuffer::Draw(GLenum mode, GLsizei first, GLsizei count) {
    glMultiDrawArraysIndirect(
        mode, 
        reinterpret_cast<void *>(static_cast<size_t>(first) * sizeof(DrawIndirectCommand)), 
        count,
        sizeof(DrawIndirectCommand)
    );
}
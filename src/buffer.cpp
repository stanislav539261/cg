#include "buffer.hpp"

void DrawIndirectBuffer::Bind() {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
}
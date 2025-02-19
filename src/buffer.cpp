#include "buffer.hpp"

void DrawIndirectBuffer::BindIndirect() const {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
}
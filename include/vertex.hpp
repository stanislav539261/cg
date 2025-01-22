#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3       m_Position;
    glm::vec2       m_Texcoord;
    glm::vec3       m_Normal;
    unsigned int    m_Material;
};

#endif /* VERTEX_HPP */
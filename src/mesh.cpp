#include "mesh.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::u32> indices) {
    m_Indices = indices;
    m_Vertices = vertices;
}

Mesh::~Mesh() {
    
}
#ifndef MESH_HPP
#define MESH_HPP

#include <vector>

#include "vertex.hpp"

class Mesh {
public:
    Mesh(std::vector<Vertex>, std::vector<unsigned int>);
    ~Mesh();

    std::vector<unsigned int>   m_Indices;
    std::vector<Vertex>         m_Vertices;
};

#endif /* MESH_HPP */
#ifndef MODEL_HPP
#define MODEL_HPP

#include <filesystem>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"

class Model {
public:
    Model(const std::filesystem::path &filename);
    ~Model();

    size_t                  NumIndices() const;
    size_t                  NumVertices() const;

    std::vector<Material>   m_Materials;
    std::vector<Mesh>       m_Meshes;
};

#endif /* MODEL_HPP */
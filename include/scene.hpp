#ifndef SCENE_HPP
#define SCENE_HPP

#include <filesystem>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"

class Scene {
public:
    Scene(const std::filesystem::path &filename);
    ~Scene();

    size_t                  NumIndices() const;
    size_t                  NumVertices() const;

    std::vector<Material>   m_Materials;
    std::vector<Mesh>       m_Meshes;
};

#endif /* SCENE_HPP */
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "model.hpp"
#include "state.hpp"

Model::Model(const std::filesystem::path &filename) {
    auto aiImport = Assimp::Importer();
    auto aiScene = aiImport.ReadFile(filename.c_str(), aiProcess_FlipUVs | aiProcess_Triangulate);

    if (aiScene && (aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0)  {
        auto materials = std::vector<Material>();
        auto meshes = std::vector<Mesh>();

        materials.reserve(aiScene->mNumMaterials);
        meshes.reserve(aiScene->mNumMeshes);

        for (auto i = 0u; i < aiScene->mNumMaterials; i++) {
            const auto aiMaterial = aiScene->mMaterials[i];

            auto diffuseImage = std::shared_ptr<Image>();
            auto metalnessImage = std::shared_ptr<Image>();
            auto normalImage = std::shared_ptr<Image>();
            auto roughnessImage = std::shared_ptr<Image>();

            for (auto j = 0u; j < std::min(aiMaterial->GetTextureCount(aiTextureType_DIFFUSE), 1u); j++) {
                aiString aiFilename;
                aiMaterial->GetTexture(aiTextureType_DIFFUSE, j, &aiFilename);
                diffuseImage = std::shared_ptr<Image>(new Image(g_ResourcePath / aiFilename.C_Str()));
            }

            for (auto j = 0u; j < std::min(aiMaterial->GetTextureCount(aiTextureType_AMBIENT), 1u); j++) {
                aiString aiFilename;
                aiMaterial->GetTexture(aiTextureType_AMBIENT, j, &aiFilename);
                metalnessImage = std::shared_ptr<Image>(new Image(g_ResourcePath / aiFilename.C_Str()));
            }

            for (auto j = 0u; j < std::min(aiMaterial->GetTextureCount(aiTextureType_HEIGHT), 1u); j++) {
                aiString aiFilename;
                aiMaterial->GetTexture(aiTextureType_HEIGHT, j, &aiFilename);
                normalImage = std::shared_ptr<Image>(new Image(g_ResourcePath / aiFilename.C_Str()));
            }

            for (auto j = 0u; j < std::min(aiMaterial->GetTextureCount(aiTextureType_SHININESS), 1u); j++) {
                aiString aiFilename;
                aiMaterial->GetTexture(aiTextureType_SHININESS, j, &aiFilename);
                roughnessImage = std::shared_ptr<Image>(new Image(g_ResourcePath / aiFilename.C_Str()));
            }

            materials.push_back(Material {
                .m_DiffuseImage = diffuseImage,
                .m_MetalnessImage = metalnessImage,
                .m_NormalImage = normalImage,
                .m_RoughnessImage = roughnessImage,
            });
        }

        for (auto i = 0u; i < aiScene->mNumMeshes; i++) {
            const auto aiMesh = aiScene->mMeshes[i];

            auto indices = std::vector<glm::u32>();
            auto vertices = std::vector<Vertex>();

            for (auto j = 0u; j < aiMesh->mNumFaces; j++) {
                const auto aiFace = &aiMesh->mFaces[j];

                for (auto k = 0u; k < aiFace->mNumIndices; k++) {
                    indices.push_back(aiFace->mIndices[k]);
                }
            }

            for (auto j = 0u; j < aiMesh->mNumVertices; j++) {
                auto position = glm::vec3(aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z);
                auto texcoord = glm::vec2(aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y);
                auto normal = glm::vec3(aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z);

                vertices.push_back(Vertex {
                    .m_Position = position,
                    .m_Texcoord = texcoord,
                    .m_Normal = normal,
                    .m_Material = aiMesh->mMaterialIndex,
                });
            }

            meshes.push_back(Mesh(vertices, indices));
        }

        m_Materials = materials;
        m_Meshes = meshes;
    } else {
        std::cout << "Assimp: " << aiImport.GetErrorString() << std::endl;
    }

    aiImport.FreeScene();
}

Model::~Model() {

}

size_t Model::NumIndices() const {
    auto accum = 0u;

    for (const auto &mesh : m_Meshes) {
        accum += mesh.m_Indices.size();
    }

    return accum;
}

size_t Model::NumVertices() const {
    auto accum = 0u;

    for (const auto &mesh : m_Meshes) {
        accum += mesh.m_Vertices.size();
    }

    return accum;
}
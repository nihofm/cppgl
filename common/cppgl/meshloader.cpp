#include "meshloader.h"
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include "file_util.h"

inline glm::vec3 to_glm(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }

std::string MeshLoader::base_path;

std::vector<std::shared_ptr<Drawelement>> MeshLoader::load(const std::string& filename, bool normalize,
        std::function<std::shared_ptr<Shader>(const std::shared_ptr<Material>&)> set_shader_callback) {
    // load from disk
    const std::string path = concat(base_path, filename);
    Assimp::Importer importer;
    std::cout << "Loading: " << path << "..." << std::endl;
    const aiScene* scene_ai = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);// | aiProcess_FlipUVs); // TODO assimp
    if (!scene_ai) // handle error
        throw std::runtime_error("ERROR: Failed to load file: " + path + "!");

    // normalize vertex positions? -> translate to origin and scale to [-1, 1]
    if (normalize) {
        // compute AABB
        glm::vec3 bb_min(FLT_MAX), bb_max(FLT_MIN);
        for (uint32_t i = 0; i < scene_ai->mNumMeshes; ++i) {
            const aiMesh* mesh_ai = scene_ai->mMeshes[i];
            for (uint32_t i = 0; i < mesh_ai->mNumVertices; ++i) {
                bb_min = glm::min(bb_min, to_glm(mesh_ai->mVertices[i]));
                bb_max = glm::max(bb_max, to_glm(mesh_ai->mVertices[i]));
            }
        }
        // compute offset to translate to origin
        const glm::vec3 center = (bb_min + bb_max) * .5f;
        // compute scale factor to scale to [-1, 1]
        const glm::vec3 min(-1), max(1);
        const glm::vec3 scale_v = (max - min) / (bb_max - bb_min);
        const float scale_f = std::min(scale_v.x, std::min(scale_v.y, scale_v.z));
        // apply
        for (uint32_t i = 0; i < scene_ai->mNumMeshes; ++i) {
            const aiMesh* mesh_ai = scene_ai->mMeshes[i];
            for (uint32_t i = 0; i < mesh_ai->mNumVertices; ++i) {
                // translate and scale
                mesh_ai->mVertices[i].x = (mesh_ai->mVertices[i].x - center.x) * scale_f;
                mesh_ai->mVertices[i].y = (mesh_ai->mVertices[i].y - center.y) * scale_f;
                mesh_ai->mVertices[i].z = (mesh_ai->mVertices[i].z - center.z) * scale_f;
            }
        }
    }

    // drawelement container
    std::vector<std::shared_ptr<Drawelement>> drawelements;

    // load materials
    std::vector<std::shared_ptr<Material>> materials;
    const std::string base_name = remove_extension(get_filename(path));
    const std::string base_path = get_basepath(path);
    for (uint32_t i = 0; i < scene_ai->mNumMaterials; ++i) {
        aiString name_ai;
        scene_ai->mMaterials[i]->Get(AI_MATKEY_NAME, name_ai);
        materials.push_back(make_material(base_name + "_" + name_ai.C_Str(), base_path, scene_ai->mMaterials[i]));
    }

    // load meshes and build drawelements
    for (uint32_t i = 0; i < scene_ai->mNumMeshes; ++i) {
        const aiMesh* ai_mesh = scene_ai->mMeshes[i];
        // build mesh and fetch corresponding material
        auto mesh = make_mesh(base_name + "_" + ai_mesh->mName.C_Str() + "_" + std::to_string(i), ai_mesh);
        auto material = materials[scene_ai->mMeshes[i]->mMaterialIndex];
        // build drawelement
        auto draw = make_drawelement(mesh->name + "_" + material->name);
        draw->add_mesh(mesh);
        draw->use_material(material);
        draw->use_shader(set_shader_callback(material));
        drawelements.push_back(draw);
    }

    return drawelements;
}

#include "material.h"
#include <iostream>

MaterialImpl::MaterialImpl() {}

MaterialImpl::MaterialImpl(const fs::path& base_path, const aiMaterial* mat_ai) {
    aiString name;
    mat_ai->Get(AI_MATKEY_NAME, name);
    aiColor3D diff, spec;
    if (mat_ai->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_DIFFUSE, 0, &path_ai);
        add_texture("diffuse", Texture2D(std::string(name.C_Str()) + "_diffuse_" + fs::path(path_ai.C_Str()).filename().string(), base_path / path_ai.C_Str()));
    } else if (mat_ai->Get(AI_MATKEY_COLOR_DIFFUSE, diff) == AI_SUCCESS) {
        // 1x1 texture with diffuse color
        add_texture("diffuse", Texture2D(std::string(name.C_Str()) + "_diffuse", 1, 1, GL_RGB32F, GL_RGB, GL_FLOAT, &diff.r));
    }
    if (mat_ai->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_SPECULAR, 0, &path_ai);
        add_texture("specular", Texture2D(std::string(name.C_Str()) + "_specular_" + fs::path(path_ai.C_Str()).filename().string(), base_path / path_ai.C_Str()));
    } else if (mat_ai->Get(AI_MATKEY_COLOR_SPECULAR, spec) == AI_SUCCESS) {
        // 1x1 texture with specular color
        add_texture("specular", Texture2D(std::string(name.C_Str()) + "_specular", 1, 1, GL_RGB32F, GL_RGB, GL_FLOAT, &spec.r));
    }
    if (mat_ai->GetTextureCount(aiTextureType_HEIGHT) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_HEIGHT, 0, &path_ai);
        add_texture("normalmap", Texture2D(std::string(name.C_Str()) + "_normalmap_" + fs::path(path_ai.C_Str()).filename().string(), base_path / path_ai.C_Str()));
    }
    if (mat_ai->GetTextureCount(aiTextureType_OPACITY) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_OPACITY, 0, &path_ai);
        add_texture("alphamap", Texture2D(std::string(name.C_Str()) + "_alphamap_" + fs::path(path_ai.C_Str()).filename().string(), base_path / path_ai.C_Str()));
    }
}

MaterialImpl::~MaterialImpl() {}

void MaterialImpl::bind(const Shader& shader) const {
    uint32_t unit = 0;
    for (const auto& entry : texture_map)
        shader->uniform(entry.first, entry.second, unit++);
}

void MaterialImpl::unbind() const {
    for (const auto& entry : texture_map)
        entry.second->unbind();
}

void MaterialImpl::add_texture(const std::string& uniform_name, const Texture2D& texture) {
    texture_map[uniform_name] = texture;
}

bool MaterialImpl::has_texture(const std::string& uniform_name) const {
    return texture_map.count(uniform_name);
}

Texture2D MaterialImpl::get_texture(const std::string& uniform_name) const {
    return texture_map.at(uniform_name);
}

#include "material.h"
#include "file_util.h"

#include <iostream>

Material::Material(const std::string& name) : NamedMap(name) {}

Material::Material(const std::string& name, const std::string& base_path, const aiMaterial* mat_ai) : Material(name) {
    const std::string old_base_path = Texture2D::base_path;
    Texture2D::base_path = base_path; // note: not thread safe
    aiColor3D diff, spec;
    if (mat_ai->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_DIFFUSE, 0, &path_ai);
        add_texture("diffuse", make_texture(name + "_diffuse_" + get_filename(path_ai.C_Str()), path_ai.C_Str()));
    } else if (mat_ai->Get(AI_MATKEY_COLOR_DIFFUSE, diff) == AI_SUCCESS) {
        // 1x1 texture with diffuse color
        add_texture("diffuse", make_texture(name + "_diffuse", 1, 1, GL_RGB32F, GL_RGB, GL_FLOAT, &diff.r));
    }
    if (mat_ai->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_SPECULAR, 0, &path_ai);
        add_texture("specular", make_texture(name + "_specular_" + get_filename(path_ai.C_Str()), path_ai.C_Str()));
    } else if (mat_ai->Get(AI_MATKEY_COLOR_SPECULAR, spec) == AI_SUCCESS) {
        // 1x1 texture with specular color
        add_texture("specular", make_texture(name + "_specular", 1, 1, GL_RGB32F, GL_RGB, GL_FLOAT, &spec.r));
    }
    if (mat_ai->GetTextureCount(aiTextureType_HEIGHT) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_HEIGHT, 0, &path_ai);
        add_texture("normalmap", make_texture(name + "_normalmap_" + get_filename(path_ai.C_Str()), path_ai.C_Str()));
    }
    if (mat_ai->GetTextureCount(aiTextureType_OPACITY) > 0) {
        aiString path_ai;
        mat_ai->GetTexture(aiTextureType_OPACITY, 0, &path_ai);
        add_texture("alphamap", make_texture(name + "_alphamap_" + get_filename(path_ai.C_Str()), path_ai.C_Str()));
    }
    Texture2D::base_path = old_base_path;
}

Material::~Material() {}

void Material::bind(const std::shared_ptr<Shader>& shader) const {
    uint32_t unit = 0;
    for (const auto& entry : texture_map)
        shader->uniform(entry.first, entry.second, unit++);
}

void Material::unbind() const {
    for (const auto& entry : texture_map)
        entry.second->unbind();
}

void Material::add_texture(const std::string& uniform_name, const std::shared_ptr<Texture2D>& texture) {
    texture_map[uniform_name] = texture;
}

bool Material::has_texture(const std::string& uniform_name) const {
    return texture_map.count(uniform_name);
}

std::shared_ptr<Texture2D> Material::get_texture(const std::string& uniform_name) const {
    return texture_map.at(uniform_name);
}

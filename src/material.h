#pragma once

#include <map>
#include <string>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <assimp/material.h>
#include "named_handle.h"
#include "shader.h"
#include "texture.h"

// ------------------------------------------
// Material

class MaterialImpl {
public:
    MaterialImpl();
    MaterialImpl(const fs::path& base_path, const aiMaterial* mat_ai);
    virtual ~MaterialImpl();

    void bind(const Shader& shader) const;
    void unbind() const;

    bool has_texture(const std::string& uniform_name) const;
    void add_texture(const std::string& uniform_name, const Texture2D& texture);
    Texture2D get_texture(const std::string& uniform_name) const;

    // data
    // TODO add more data: int/float/vec2/vec3/vec4
    std::map<std::string, Texture2D> texture_map;
};

using Material = NamedHandle<MaterialImpl>;

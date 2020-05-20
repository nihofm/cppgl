#pragma once

#include <map>
#include <string>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;
#include <functional>
#include "mesh.h"
#include "material.h"
#include "shader.h"
#include "drawelement.h"

// if normalize == true: translate mesh to origin and scale vertices to fit [-1, 1]
// use the set_shader_callback to set shaders depending on the material (f.e. with/-out normalmap)

class MeshLoader {
public:
    static std::vector<std::shared_ptr<Drawelement>> load(const fs::path& path, bool normalize = false,
            std::function<std::shared_ptr<Shader>(const std::shared_ptr<Material>&)> set_shader_callback =
            [](const std::shared_ptr<Material>&) { return std::shared_ptr<Shader>(); });
};

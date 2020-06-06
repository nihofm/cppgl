#pragma once

#include "named_map.h"
#include "mesh.h"
#include "shader.h"

// -----------------------------------------------
// Drawelement (object instance for rendering)

class Drawelement : public NamedMap<Drawelement> {
public:
    Drawelement(const std::string& name);
    Drawelement(const std::string& name, const Shader& shader);
    Drawelement(const std::string& name, const Shader& shader, const Mesh& mesh);
    virtual ~Drawelement();

    void bind() const;
    void draw() const;
    void unbind() const;

    glm::mat4 model;
    Shader shader;
    Mesh mesh;
};

// variadic alias for std::make_shared<>(...)
template <class... Args> std::shared_ptr<Drawelement> make_drawelement(Args&&... args) {
    return std::make_shared<Drawelement>(args...);
}

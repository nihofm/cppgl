#pragma once

#include "named_handle.h"
#include "mesh.h"
#include "shader.h"

// -----------------------------------------------
// Drawelement (object instance for rendering)

class DrawelementImpl {
public:
    DrawelementImpl(const MeshPtr& mesh = MeshPtr(), const ShaderPtr& shader = ShaderPtr());
    virtual ~DrawelementImpl();

    void bind() const;
    void draw() const;
    void unbind() const;

    glm::mat4 model;
    ShaderPtr shader;
    MeshPtr mesh;
};

using DrawelementPtr = NamedHandle<DrawelementImpl>;
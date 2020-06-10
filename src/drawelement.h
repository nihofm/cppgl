#pragma once

#include "named_handle.h"
#include "mesh.h"
#include "shader.h"

// -----------------------------------------------
// Drawelement (object instance for rendering)

class DrawelementImpl {
public:
    DrawelementImpl(const Mesh& mesh = Mesh(), const Shader& shader = Shader());
    virtual ~DrawelementImpl();

    void bind() const;
    void draw() const;
    void unbind() const;

    glm::mat4 model;
    Shader shader;
    Mesh mesh;
};

using Drawelement = NamedHandle<DrawelementImpl>;
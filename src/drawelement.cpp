#include "drawelement.h"
#include "camera.h"
#include <iostream>

Drawelement::Drawelement(const std::string& name) : NamedMap(name), model(glm::mat4(1)) {}

Drawelement::Drawelement(const std::string& name, const Shader& shader) : Drawelement(name) {
    this->shader = shader;
}

Drawelement::Drawelement(const std::string& name, const Shader& shader, const std::shared_ptr<Mesh>& mesh) : Drawelement(name) {
    this->shader = shader;
    this->mesh = mesh;
}

Drawelement::~Drawelement() {}

void Drawelement::bind() const {
    if (shader) {
        shader->bind();
        if (mesh) mesh->bind(shader);
        shader->uniform("model", model);
        shader->uniform("view", Camera::current()->view);
        shader->uniform("view_normal", Camera::current()->view_normal);
        shader->uniform("proj", Camera::current()->proj);
    }
}

void Drawelement::unbind() const {
    if (mesh) mesh->unbind();
    if (shader) shader->unbind();
}

void Drawelement::draw() const {
    mesh->draw();
}

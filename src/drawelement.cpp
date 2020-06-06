#include "drawelement.h"
#include "camera.h"
#include <iostream>

Drawelement::Drawelement(const std::string& name) : NamedMap(name), model(glm::mat4(1)) {}

Drawelement::Drawelement(const std::string& name, const Shader& shader) : Drawelement(name) {
    this->shader = shader;
}

Drawelement::Drawelement(const std::string& name, const Shader& shader, const Mesh& mesh) : Drawelement(name) {
    this->shader = shader;
    this->mesh = mesh;
}

Drawelement::~Drawelement() {}

void Drawelement::bind() const {
    if (shader) {
        shader->bind();
        if (mesh) mesh->bind(shader);
        shader->uniform("model", model);
        shader->uniform("view", current_camera()->view);
        shader->uniform("view_normal", current_camera()->view_normal);
        shader->uniform("proj", current_camera()->proj);
    }
}

void Drawelement::unbind() const {
    if (mesh) mesh->unbind();
    if (shader) shader->unbind();
}

void Drawelement::draw() const {
    mesh->draw();
}

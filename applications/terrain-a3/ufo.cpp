#include "ufo.h"
#include "rendering.h"
#include <glm/gtc/matrix_transform.hpp>

std::vector<Drawelement> Ufo::prototype;

Ufo::Ufo() {
    // lazy init prototype
    if (prototype.empty()) {
        auto shader = Shader("ufo", "shader/ufo.vs", "shader/ufo.fs");
        auto meshes = load_meshes_gpu("render-data/ufo/ufo.obj", true);
        for (auto m : meshes) {
            Ufo::prototype.push_back(Drawelement(m->name, shader, m));
        }
    }
    // rotate and translate ufo
    model = glm::rotate(glm::mat4(1), float(M_PI) * -.5f, glm::vec3(1, 0, 0));
    model[3][0] = 3;
}

Ufo::~Ufo() {}

void Ufo::draw() {
    for (auto& elem : prototype) {
        elem->model = model;
        elem->bind();
        setup_light(elem->shader);
        elem->draw();
        elem->unbind();
    }
}

#include "ufo.h"
#include "rendering.h"
#include <glm/gtc/matrix_transform.hpp>

std::vector<std::shared_ptr<Drawelement>> Ufo::prototype;

Ufo::Ufo() {
    // lazy init prototype
    if (prototype.empty()) {
        auto shader = make_shader("ufo", "shader/ufo.vs", "shader/ufo.fs");
        auto shader_callback = [shader](const std::shared_ptr<Material>&) { return shader; };
        prototype = MeshLoader::load("render-data/ufo/ufo.obj", true, shader_callback);
    }
    // rotate and translate ufo
    trafo = glm::rotate(glm::mat4(1), float(M_PI) * -.5f, glm::vec3(1, 0, 0));
    trafo[3][0] = 3;
}

Ufo::~Ufo() {}

void Ufo::draw() {
    for (auto& elem : prototype) {
        elem->bind();
        setup_light(elem->shader);
        elem->draw(trafo);
        elem->unbind();
    }
}

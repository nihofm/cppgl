#include "rendering.h"
#include <cppgl/quad.h>
#include <cppgl/camera.h>

using namespace std;

void setup_light(const std::shared_ptr<Shader>& shader) {
	shader->uniform("ambient_col", glm::vec3(0.12f, 0.14f, 0.16f));
	shader->uniform("light_dir", glm::normalize(glm::vec3(1.f, -0.6f, -0.4f)));
	shader->uniform("light_col", glm::vec3(0.6f, 0.7f, 0.8f));
    shader->uniform("cam_pos", Camera::current()->pos);
}

void blit(const std::shared_ptr<Texture2D>& tex) {
    blit(*tex);
}

void blit(const Texture2D& tex) {
	static auto shader = make_shader("blit", "shader/copytex.vs", "shader/copytex.fs");
	shader->bind();
    shader->uniform("tex", tex, 0);
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}

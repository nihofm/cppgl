#include "rendering.h"
#include <cppgl/quad.h>
#include <cppgl/timer.h>

using namespace std;

float render_settings::tile_size = 10;
float render_settings::character_radius = tile_size * 0.45f;
float render_settings::character_float_h = character_radius * 1.75f;
glm::vec2 render_settings::character_camera_offset = glm::vec2(tile_size * 4, tile_size * 5);
float render_settings::bomb_radius = tile_size * 0.3f;
int render_settings::particle_emitter_timeslice = int(Timer::msec(150));
float render_settings::particle_size = 4;
float render_settings::camera_shake_duration = 1000.f;
float render_settings::box_explosion_duration = 2000.f;

void setup_light(const std::shared_ptr<Shader>& shader) {
	shader->uniform("ambient_col", glm::vec3(0.12f, 0.14f, 0.16f));
	shader->uniform("light_dir", glm::normalize(glm::vec3(1.f, -0.6f, -0.4f)));
	shader->uniform("light_col", glm::vec3(0.6f, 0.7f, 0.8f));
}

void blit(const std::shared_ptr<Texture2D>& tex) {
    blit(*tex);
}

void blit(const Texture2D& tex) {
	static std::shared_ptr<Shader> shader = make_shader("blit", "shader/copytex.vs", "shader/copytex.fs");
	shader->bind();
    shader->uniform("tex", tex, 0);
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}

void blit_depth(const std::shared_ptr<Texture2D>& depth, const std::shared_ptr<Texture2D>& col) {
    blit_depth(*depth, *col);
}

void blit_depth(const Texture2D& depth, const Texture2D& col) {
	static std::shared_ptr<Shader> shader = make_shader("blit_depth", "shader/copytex.vs", "shader/copytex-depth.fs");
	shader->bind();
    shader->uniform("depth", depth, 0);
    shader->uniform("col", col, 1);
    Quad::draw();
    shader->unbind();
}

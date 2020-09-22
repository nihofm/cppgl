#include "rendering.h"
#include <cppgl.h>

using namespace std;

float render_settings::tile_size = 10;
float render_settings::character_radius = tile_size * 0.45f;
float render_settings::character_float_h = character_radius * 1.75f;
glm::vec2 render_settings::character_camera_offset = glm::vec2(tile_size * 4, tile_size * 5);
float render_settings::bomb_radius = tile_size * 0.3f;
int render_settings::particle_emitter_timeslice = int(150);
float render_settings::particle_size = 4;
float render_settings::camera_shake_duration = 1000.f;
float render_settings::box_explosion_duration = 2000.f;

void setup_light(const Shader& shader) {
	shader->uniform("ambient_col", glm::vec3(0.12f, 0.14f, 0.16f));
	shader->uniform("light_dir", glm::normalize(glm::vec3(1.f, -0.6f, -0.4f)));
	shader->uniform("light_col", glm::vec3(0.6f, 0.7f, 0.8f));
}

void deferred_debug_pass(const Framebuffer& gbuffer) {
    static auto shader = Shader("deferred-debug", "shader/copytex.vs", "shader/deferred-debug.fs");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->bind();
    setup_light(shader);
    shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
    shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
    shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
    shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
    shader->uniform("near_far", glm::vec2(current_camera()->near, current_camera()->far));
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}

void deferred_shading_pass(const Framebuffer& gbuffer) {
    static auto shader = Shader("deferred-lighting", "shader/copytex.vs", "shader/deferred-lighting.fs");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->bind();
    setup_light(shader);
    shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
    shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
    shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
    shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
    shader->uniform("near_far", glm::vec2(current_camera()->near, current_camera()->far));
    shader->uniform("screenres", glm::vec2(Context::resolution()));
    Quad::draw();
    shader->unbind();
}

void blit(const std::shared_ptr<Texture2D>& tex) {
    blit(*tex);
}

void blit(const Texture2D& tex) {
	static auto shader = Shader("blit", "shader/copytex.vs", "shader/copytex.fs");
	shader->bind();
    shader->uniform("tex", tex, 0);
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}

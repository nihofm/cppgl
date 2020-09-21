#include "particles.h"
#include <cppgl/context.h>
#include <cppgl/camera.h>
#include <cppgl/texture.h>

#include <iostream>

std::shared_ptr<Shader> Particles::draw_shader;
std::shared_ptr<Shader> Particles::splat_shader;

Particles::Particles(unsigned int N, float particle_size) : N(N), position(N), direction(N), lifetime(N, 0), index_buffer(N), start(0), end(0), particle_size(particle_size) {
    static unsigned idx = 0;
    mesh = make_mesh(std::string("particle-data") + std::to_string(idx++));
    vbo_id_pos = mesh->add_vertex_buffer(GL_FLOAT, 3, N, position.data(), GL_DYNAMIC_DRAW);
    vbo_id_life = mesh->add_vertex_buffer(GL_INT, 1, N, lifetime.data(), GL_DYNAMIC_DRAW);
    for (unsigned i = 0; i < N; ++i) index_buffer[i] = i;
    mesh->add_index_buffer(N, index_buffer.data());
    // lazy init static stuff
    if (!draw_shader)
        draw_shader = make_shader("particle-draw", "shader/particle-flare.vs", "shader/particle-flare.fs");
    if (!splat_shader)
        splat_shader = make_shader("particle-splat", "shader/deferred-splats.vs", "shader/deferred-splats.gs", "shader/deferred-splats.fs");
}

Particles::~Particles() {}

uint32_t Particles::advance(uint32_t& pointer) {
    pointer = (pointer + 1) % N;
    return pointer;
}

void Particles::add(const glm::vec3& pos, const glm::vec3& dir, int alive_for_ms) {
    const uint32_t at = end;
    if (advance(end) == start)
        advance(start);
    position[at] = pos;
    direction[at] = dir;
    lifetime[at] = alive_for_ms;
}

void Particles::update() {
    const float dt = Context::frame_time();
    const int bound = (end >= start) ? end : end+N;
    for (int i = start; i < bound; ++i) {
        const uint32_t idx = i % N;
        lifetime[idx] -= uint32_t(dt);
        if (lifetime[idx] <= 0 && idx == start)
            advance(start);
        position[idx] = position[idx] + direction[idx] * dt * 0.0066f;
    }
    mesh->update_vertex_buffer(vbo_id_pos, position.data());
    mesh->update_vertex_buffer(vbo_id_life, lifetime.data());
}

void Particles::draw() {
    // draw pass as points
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    draw_shader->bind();
    auto cam = Camera::current();
    draw_shader->uniform("view", cam->view);
    draw_shader->uniform("proj", cam->proj);
    draw_shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    draw_shader->uniform("screenres", glm::vec2(Context::resolution()));
    draw_shader->uniform("particle_size", particle_size);
    draw_shader->uniform("depth_tex", Texture2D::find("gbuf_depth"), 0);

    mesh->bind();
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
    }
    mesh->unbind();
    draw_shader->unbind();

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_PROGRAM_POINT_SIZE);

    // splatting pass using a geometry shader
    splat_shader->bind();
    splat_shader->uniform("view", cam->view);
    splat_shader->uniform("proj", cam->proj);
    splat_shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    splat_shader->uniform("screenres", glm::vec2(Context::resolution()));
    splat_shader->uniform("radius", particle_size * 2.5f);
    splat_shader->uniform("gbuf_depth", Texture2D::find("gbuf_depth"), 0);
    splat_shader->uniform("gbuf_diff", Texture2D::find("gbuf_diff"), 1);
    splat_shader->uniform("gbuf_pos", Texture2D::find("gbuf_pos"), 2);
    splat_shader->uniform("gbuf_norm", Texture2D::find("gbuf_norm"), 3);

    mesh->bind();
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
    }
    mesh->unbind();

    splat_shader->unbind();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

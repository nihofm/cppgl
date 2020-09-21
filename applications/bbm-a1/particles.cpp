#include "particles.h"
#include <cppgl/context.h>
#include <cppgl/camera.h>

#include <iostream>
using namespace std;

std::shared_ptr<Shader> Particles::shader;

Particles::Particles(unsigned int N, float particle_size) : N(N), position(N), direction(N), lifetime(N, 0), index_buffer(N), start(0), end(0), particle_size(particle_size) {
    static unsigned idx = 0;
    mesh = make_mesh(std::string("particle-data") + std::to_string(idx++));
    vbo_id_pos = mesh->add_vertex_buffer(GL_FLOAT, 3, N, position.data(), GL_DYNAMIC_DRAW);
    vbo_id_life = mesh->add_vertex_buffer(GL_INT, 1, N, lifetime.data(), GL_DYNAMIC_DRAW);
    for (unsigned i = 0; i < N; ++i) index_buffer[i] = i;
    mesh->add_index_buffer(N, index_buffer.data());
    // lazy init static stuff
    if (!shader)
        shader = make_shader("particle-shader", "shader/particle-flare.vs", "shader/particle-flare.fs");
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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    shader->bind();
    auto cam = Camera::current();
    shader->uniform("view", cam->view);
    shader->uniform("proj", cam->proj);
    shader->uniform("screenres", glm::vec2(Context::resolution()));
    shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    shader->uniform("particle_size", particle_size);
    shader->uniform("depth_tex", Texture2D::find("fbo_depth"), 0);

    mesh->bind();
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
    }
    mesh->unbind();
    shader->unbind();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_PROGRAM_POINT_SIZE);
}


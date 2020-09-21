#pragma once

#include <cppgl/mesh.h>
#include <cppgl/shader.h>

class Particles {
    const uint32_t N;
    std::vector<glm::vec3> position;
    std::vector<glm::vec3> direction;
    std::vector<int> lifetime;
    std::vector<uint32_t> index_buffer;
    uint32_t vbo_id_pos, vbo_id_life;
    uint32_t start, end;
    const float particle_size;
    std::shared_ptr<Mesh> mesh;
    static std::shared_ptr<Shader> draw_shader;
    static std::shared_ptr<Shader> splat_shader;

    uint32_t advance(uint32_t& pointer);

public:
    Particles(unsigned int N, float particle_size);
    ~Particles();

    void add(const glm::vec3& pos, const glm::vec3& dir, int alive_for_ms);
    void update();
    void draw();
};

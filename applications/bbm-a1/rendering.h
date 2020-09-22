#pragma once

#include <cppgl.h>

struct render_settings {
	static float tile_size;
	static float character_radius;
	static float character_float_h;
    static glm::vec2 character_camera_offset; // vec2(dist_horizontal, dist_vertical)
	static float bomb_radius;
	static int particle_emitter_timeslice;
	static float particle_size;
    static float camera_shake_duration;
    static float box_explosion_duration;
};

// draw random float in [-1, 1]
inline float random_float() { return (float(rand() % 32768) / 16384.0f) - 1.0f; }
inline glm::vec2 random_vec2() { return glm::vec2(random_float(), random_float()); }
inline glm::vec3 random_vec3() { return glm::vec3(random_float(), random_float(), random_float()); }

void setup_light(const Shader& shader);

void blit(const Texture2D& tex);
void blit(const std::shared_ptr<Texture2D>& tex);
void blit_depth(const Texture2D& depth, const Texture2D& col);
void blit_depth(const std::shared_ptr<Texture2D>& depth, const std::shared_ptr<Texture2D>& col);

#pragma once

#include <cppgl/shader.h>
#include <cppgl/texture.h>

// draw random floats in [-1, 1]
inline float random_float() { return (float(rand() % 32768) / 16384.0f) - 1.0f; }
inline glm::vec2 random_vec2() { return glm::vec2(random_float(), random_float()); }
inline glm::vec3 random_vec3() { return glm::vec3(random_float(), random_float(), random_float()); }

void setup_light(const std::shared_ptr<Shader>& shader);

void blit(const Texture2D& tex);
void blit(const std::shared_ptr<Texture2D>& tex);

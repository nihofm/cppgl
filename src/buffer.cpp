#include "buffer.h"

// ----------------------------------------------------
// Shader storage buffer (std430)

SSBO::SSBO(const std::string& name) : NamedMap(name), size_bytes(0) {
    glGenBuffers(1, &id);
}

SSBO::SSBO(const std::string& name, size_t size_bytes) : SSBO(name) {
    resize(size_bytes);
}

SSBO::~SSBO() {
    glDeleteBuffers(1, &id);
}

void SSBO::resize(size_t size_bytes, GLenum hint) {
    this->size_bytes = size_bytes;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size_bytes, 0, hint);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::bind(uint32_t unit) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, id);
}

void SSBO::unbind(uint32_t unit) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, 0);
}

void* SSBO::map(GLenum access) const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    return glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
}

void SSBO::unmap() const {
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// ----------------------------------------------------
// Uniform buffer (std140)

UBO::UBO(const std::string& name) : NamedMap(name), size_bytes(0) {
    glGenBuffers(1, &id);
}

UBO::UBO(const std::string& name, size_t size_bytes) : UBO(name) {
    resize(size_bytes);
}

UBO::~UBO() {
    glDeleteBuffers(1, &id);
}

void UBO::resize(size_t size_bytes, GLenum hint) {
    this->size_bytes = size_bytes;
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, size_bytes, 0, hint);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::bind(uint32_t unit) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, id);
}

void UBO::unbind(uint32_t unit) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0);
}

void* UBO::map(GLenum access) const {
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    return glMapBuffer(GL_UNIFORM_BUFFER, access);
}

void UBO::unmap() const {
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

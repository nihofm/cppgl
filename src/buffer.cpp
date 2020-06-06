#include "buffer.h"

// ----------------------------------------------------
// Shader storage buffer (std430)

SSBOImpl::SSBOImpl(size_t size_bytes) {
    glGenBuffers(1, &id);
    resize(size_bytes);
}

SSBOImpl::~SSBOImpl() {
    glDeleteBuffers(1, &id);
}

void SSBOImpl::resize(size_t size_bytes, GLenum hint) {
    this->size_bytes = size_bytes;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size_bytes, 0, hint);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBOImpl::bind(uint32_t unit) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, id);
}

void SSBOImpl::unbind(uint32_t unit) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, 0);
}

void* SSBOImpl::map(GLenum access) const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    return glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
}

void SSBOImpl::unmap() const {
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// ----------------------------------------------------
// Uniform buffer (std140)

UBOImpl::UBOImpl(size_t size_bytes) {
    glGenBuffers(1, &id);
    resize(size_bytes);
}

UBOImpl::~UBOImpl() {
    glDeleteBuffers(1, &id);
}

void UBOImpl::resize(size_t size_bytes, GLenum hint) {
    this->size_bytes = size_bytes;
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, size_bytes, 0, hint);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOImpl::bind(uint32_t unit) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, id);
}

void UBOImpl::unbind(uint32_t unit) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0);
}

void* UBOImpl::map(GLenum access) const {
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    return glMapBuffer(GL_UNIFORM_BUFFER, access);
}

void UBOImpl::unmap() const {
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

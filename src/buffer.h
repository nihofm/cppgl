#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include "named_handle.h"

// ----------------------------------------------------
// SSBO

class SSBOImpl;
using SSBO = NamedHandle<SSBOImpl>;

// ----------------------------------------------------
// Shader storage buffer implementation (std430, binding = unit)

class SSBOImpl {
public:
    SSBOImpl(size_t size_bytes = 0);
    virtual ~SSBOImpl();

    // prevent copies and moves, since GL buffers aren't reference counted
    SSBOImpl(const SSBOImpl&) = delete;
    SSBOImpl& operator=(const SSBOImpl&) = delete;
    SSBOImpl& operator=(const SSBOImpl&&) = delete;

    explicit inline operator bool() const { return glIsBuffer(id) && size_bytes > 0; }
    inline operator GLuint() const { return id; }

    // resize (discards all data!)
    void resize(size_t size_bytes, GLenum hint = GL_DYNAMIC_DRAW);

    // bind/unbind to/from OpenGL
    void bind(uint32_t unit) const;
    void unbind(uint32_t unit) const;

    // map/unmap from GPU mem (https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf)
    void* map(GLenum access = GL_READ_WRITE) const;
    void unmap() const;

    // data
    GLuint id;
    size_t size_bytes;
};

// ----------------------------------------------------
// UBO

class UBOImpl;
using UBO = NamedHandle<UBOImpl>;

// ----------------------------------------------------
// Uniform buffer implementation (std140, binding = unit) TODO test

class UBOImpl {
public:
    UBOImpl(size_t size_bytes);
    virtual ~UBOImpl();

    // prevent copies and moves, since GL buffers aren't reference counted
    UBOImpl(const UBOImpl&) = delete;
    UBOImpl& operator=(const UBOImpl&) = delete;
    UBOImpl& operator=(const UBOImpl&&) = delete;

    explicit inline operator bool() const { return glIsBuffer(id) && size_bytes > 0; }
    inline operator GLuint() const { return id; }

    // resize (discards all data!)
    void resize(size_t size_bytes, GLenum hint = GL_DYNAMIC_DRAW);

    // bind/unbind to/from OpenGL
    void bind(uint32_t unit) const;
    void unbind(uint32_t unit) const;

    // map/unmap from GPU mem (https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf)
    void* map(GLenum access = GL_READ_WRITE) const;
    void unmap() const;

    // data
    GLuint id;
    size_t size_bytes;
};

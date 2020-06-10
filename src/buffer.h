#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include "named_handle.h"

// ----------------------------------------------------
// Generic GLBufferImpl

template <GLenum GL_TEMPLATE_BUFFER> class GLBufferImpl {
public:
    GLBufferImpl(size_t size_bytes = 0) {
        glGenBuffers(1, &id);
        resize(size_bytes);
    }
    virtual ~GLBufferImpl() {
        glDeleteBuffers(1, &id);
    }

    // prevent copies and moves, since GL buffers aren't reference counted
    GLBufferImpl(const GLBufferImpl&) = delete;
    GLBufferImpl& operator=(const GLBufferImpl&) = delete;
    GLBufferImpl& operator=(const GLBufferImpl&&) = delete;

    explicit inline operator bool() const { return glIsBuffer(id) && size_bytes > 0; }
    inline operator GLuint() const { return id; }

    // resize (discards all data!)
    void resize(size_t size_bytes, GLenum hint = GL_DYNAMIC_DRAW) {
        this->size_bytes = size_bytes;
        glBindBuffer(GL_TEMPLATE_BUFFER, id);
        glBufferData(GL_TEMPLATE_BUFFER, size_bytes, 0, hint);
        glBindBuffer(GL_TEMPLATE_BUFFER, 0);
    }

    // bind/unbind to/from OpenGL
    void bind() const {
        glBindBuffer(GL_TEMPLATE_BUFFER, id);
    }
    void unbind() const {
        glBindBuffer(GL_TEMPLATE_BUFFER, 0);
    }
    void bind_base(uint32_t unit) const {
        glBindBufferBase(GL_TEMPLATE_BUFFER, unit, id);
    }
    void unbind_base(uint32_t unit) const {
        glBindBufferBase(GL_TEMPLATE_BUFFER, unit, 0);
    }

    // map/unmap from GPU mem (https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf)
    void* map(GLenum access = GL_READ_WRITE) const {
        bind();
        return glMapBuffer(GL_TEMPLATE_BUFFER, access);
    }
    void unmap() const {
        glUnmapBuffer(GL_TEMPLATE_BUFFER);
        unbind();
    }

    // data
    GLuint id;
    size_t size_bytes;
};

// ----------------------------------------------------
// Specialized GL buffers

using VBOPtr = NamedHandle<GLBufferImpl<GL_ARRAY_BUFFER>>;
using IBOPtr = NamedHandle<GLBufferImpl<GL_ELEMENT_ARRAY_BUFFER>>;
using UBOPtr = NamedHandle<GLBufferImpl<GL_UNIFORM_BUFFER>>;
using SSBOPtr = NamedHandle<GLBufferImpl<GL_SHADER_STORAGE_BUFFER>>;
using TBOPtr = NamedHandle<GLBufferImpl<GL_TEXTURE_BUFFER>>;
using QBOPtr = NamedHandle<GLBufferImpl<GL_QUERY_BUFFER>>;
using ACBOPtr = NamedHandle<GLBufferImpl<GL_ATOMIC_COUNTER_BUFFER>>;
using DIBOPtr = NamedHandle<GLBufferImpl<GL_DRAW_INDIRECT_BUFFER>>;
using CIBOPtr = NamedHandle<GLBufferImpl<GL_DISPATCH_INDIRECT_BUFFER>>;
using TFBOPtr = NamedHandle<GLBufferImpl<GL_TRANSFORM_FEEDBACK_BUFFER>>;
using PUBOPtr = NamedHandle<GLBufferImpl<GL_PIXEL_UNPACK_BUFFER>>;
using PPBOPtr = NamedHandle<GLBufferImpl<GL_PIXEL_PACK_BUFFER>>;
using CRBOPtr = NamedHandle<GLBufferImpl<GL_COPY_READ_BUFFER>>;
using CWBOPtr = NamedHandle<GLBufferImpl<GL_COPY_WRITE_BUFFER>>;
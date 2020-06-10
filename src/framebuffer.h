#pragma once

#include <string>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <GL/gl.h>
#include "named_handle.h"
#include "texture.h"

// ------------------------------------------
// Framebuffer

class FramebufferImpl {
public:
    FramebufferImpl(uint32_t w, uint32_t h);
    virtual ~FramebufferImpl();

    // prevent copies and moves, since GL buffers aren't reference counted
    FramebufferImpl(const FramebufferImpl&) = delete;
    FramebufferImpl& operator=(const FramebufferImpl&) = delete;
    FramebufferImpl& operator=(const FramebufferImpl&&) = delete; // TODO allow moves?

    inline operator GLuint() const { return id; }

    void bind();
    void unbind();

    void check() const;
    void resize(uint32_t w, uint32_t h);

    void attach_depthbuffer(Texture2DPtr tex = Texture2DPtr());
    void attach_colorbuffer(const Texture2DPtr& tex);

    // data
    GLuint id;
    uint32_t w, h;
    std::vector<Texture2DPtr> color_textures;
    std::vector<GLenum> color_targets;
    Texture2DPtr depth_texture;
    GLint prev_vp[4];
};

using FramebufferPtr = NamedHandle<FramebufferImpl>;
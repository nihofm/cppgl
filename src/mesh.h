#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "named_handle.h"
#include "geometry.h"
#include "material.h"

// ------------------------------------------
// Mesh

class MeshImpl {
public:
    MeshImpl(const GeometryPtr& geometry = GeometryPtr(), const MaterialPtr& material = MaterialPtr());
    virtual ~MeshImpl();

    // prevent copies and moves, since GL buffers aren't reference counted
    MeshImpl(const MeshImpl&) = delete;
    MeshImpl& operator=(const MeshImpl&) = delete;
    MeshImpl& operator=(const MeshImpl&&) = delete;

    // TODO cleaner mesh class/interface?
    void clear_gpu(); // free gpu resources
    void upload_gpu(); // cpu -> gpu transfer

    void bind(const ShaderPtr& shader) const;
    void draw() const;
    void unbind() const;

    // GL vertex and index buffer operations
    uint32_t add_vertex_buffer(GLenum type, uint32_t element_dim, uint32_t num_vertices, const void* data, GLenum hint = GL_STATIC_DRAW);
    void add_index_buffer(uint32_t num_indices, const uint32_t* data, GLenum hint = GL_STATIC_DRAW);
    void update_vertex_buffer(uint32_t buf_id, const void* data); // assumes matching size for buffer buf_id from add_vertex_buffer()
    void set_primitive_type(GLenum type); // default: GL_TRIANGLES

    // map/unmap from GPU mem (https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf)
    void* map_vbo(uint32_t buf_id, GLenum access = GL_READ_WRITE) const;
    void unmap_vbo(uint32_t buf_id) const;
    void* map_ibo(GLenum access = GL_READ_WRITE) const;
    void unmap_ibo() const;

    // CPU data
    GeometryPtr geometry;
    MaterialPtr material;
    // GPU data
    GLuint vao, ibo;
    uint32_t num_vertices;
    uint32_t num_indices;
    std::vector<GLuint> vbo_ids;
    std::vector<GLenum> vbo_types;
    std::vector<uint32_t> vbo_dims;
    GLenum primitive_type;
};

using MeshPtr = NamedHandle<MeshImpl>;

// TODO move this to MeshPtr::load()?
std::vector<MeshPtr> load_meshes(const fs::path& path, bool normalize = false);
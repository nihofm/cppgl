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

class MeshImpl;
using Mesh = NamedHandle<MeshImpl>;

std::vector<Mesh> load_meshes(const fs::path& path);

// ------------------------------------------
// MeshImpl

class MeshImpl {
public:
    // construct empty mesh
    MeshImpl();
    // construct from geometry
    MeshImpl(const Geometry& geometry);
    // construct from geometry and material
    MeshImpl(const Geometry& geometry, const Material& material);
    virtual ~MeshImpl();

    // TODO refactor
    // load mesh data from disk via assimp (obj, ply, ...)
    static std::vector<Mesh> load(const fs::path& path);

    // prevent copies and moves, since GL buffers aren't reference counted
    MeshImpl(const MeshImpl&) = delete;
    MeshImpl& operator=(const MeshImpl&) = delete;
    MeshImpl& operator=(const MeshImpl&&) = delete;

    // TODO cleaner mesh class/interface?
    void clear_gpu(); // free gpu resources
    void upload_gpu(); // cpu -> gpu transfer

    void bind(const Shader& shader) const;
    void draw() const;
    void unbind() const;

    // GL vertex and index buffer operations
    uint32_t add_vertex_buffer(GLenum type, uint32_t element_dim, uint32_t num_vertices, const void* data, GLenum hint = GL_STATIC_DRAW);
    void add_index_buffer(uint32_t num_indices, const uint32_t* data, GLenum hint = GL_STATIC_DRAW);
    void update_vertex_buffer(uint32_t buf_id, const void* data); // assumes matching size for buffer buf_id from add_vertex_buffer()
    void set_primitive_type(GLenum type); // default: GL_TRIANGLES

    // CPU data
    Geometry geometry;
    Material material;
    // GPU data
    GLuint vao, ibo;
    uint32_t num_vertices;
    uint32_t num_indices;
    std::vector<GLuint> vbo_ids;
    std::vector<GLenum> vbo_types;
    std::vector<uint32_t> vbo_dims;
    GLenum primitive_type;
};

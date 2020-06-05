#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "named_map.h"
#include "geometry.h"
#include "material.h"

// ------------------------------------------
// Mesh class

class Mesh : public NamedMap<Mesh> {
public:
    // construct empty mesh
    Mesh(const std::string& name);
    // construct from geometry
    Mesh(const std::string& name, const std::shared_ptr<Geometry>& geometry);
    // construct from geometry and material
    Mesh(const std::string& name, const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<Material>& material);
    virtual ~Mesh();

    // load mesh data from disk via assimp (obj, ply, ...)
    static std::vector<std::shared_ptr<Mesh>> load(const fs::path& path);

    // prevent copies and moves, since GL buffers aren't reference counted
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(const Mesh&&) = delete;

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
    std::shared_ptr<Geometry> geometry;
    std::shared_ptr<Material> material;
    // GPU data
    GLuint vao, ibo;
    uint32_t num_vertices;
    uint32_t num_indices;
    std::vector<GLuint> vbo_ids;
    std::vector<GLenum> vbo_types;
    std::vector<uint32_t> vbo_dims;
    GLenum primitive_type;
};

// variadic alias for std::make_shared<>(...)
template <class... Args> std::shared_ptr<Mesh> make_mesh(Args&&... args) {
    return std::make_shared<Mesh>(args...);
}

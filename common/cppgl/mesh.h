#pragma once

#include <string>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <assimp/mesh.h>
#include "named_map.h"

// ------------------------------------------
// Mesh class

class Mesh : public NamedMap<Mesh> {
public:
    // construct empty mesh
    Mesh(const std::string& name);
    // construct from assimp mesh
    Mesh(const std::string& name, const aiMesh* mesh_ai);
    // construct from triangle soup
    Mesh(const std::string& name,
            const std::vector<glm::vec3>& positions,
            const std::vector<uint32_t>& indices,
            const std::vector<glm::vec3>& normals = std::vector<glm::vec3>(),
            const std::vector<glm::vec2>& texcoords = std::vector<glm::vec2>());
    virtual ~Mesh();

    // prevent copies and moves, since GL buffers aren't reference counted
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(const Mesh&&) = delete;

    void bind() const;
    void unbind() const;

    void draw() const;

    // vertex and index buffer operations
    uint32_t add_vertex_buffer(GLenum type, uint32_t element_dim, uint32_t num_vertices, const void* data, GLenum hint = GL_STATIC_DRAW);
    void add_index_buffer(uint32_t num_indices, const uint32_t* data, GLenum hint = GL_STATIC_DRAW);
    void update_vertex_buffer(uint32_t buf_id, const void* data); // assumes matching size for buffer buf_id from add_vertex_buffer()

    // set primitive type for drawing this mesh (default: GL_TRIANGLES)
    void set_primitive_type(GLenum type);

    // data
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

#include "mesh.h"
#include <iostream>

// ------------------------------------------
// helper funcs

inline uint32_t type_to_bytes(GLenum type) {
    switch (type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_HALF_FLOAT:
        return 2;
    case GL_FLOAT:
    case GL_FIXED:
    case GL_INT:
    case GL_UNSIGNED_INT:
        return 4;
    case GL_DOUBLE:
        return 8;
    default:
        throw std::runtime_error("Unknown GL type!");
    }
}

inline glm::vec3 to_glm(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }

// ------------------------------------------
// Mesh class

Mesh::Mesh(const std::string& name) : NamedMap(name), vao(0), ibo(0), num_vertices(0), num_indices(0), primitive_type(GL_TRIANGLES) {
    glGenVertexArrays(1, &vao);
}

Mesh::Mesh(const std::string& name, const aiMesh *mesh_ai) : Mesh(name) {
    // extract vertices, normals and texture coords
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    positions.reserve(mesh_ai->mNumVertices);
    normals.reserve(mesh_ai->HasNormals() ? mesh_ai->mNumVertices : 0);
    texcoords.reserve(mesh_ai->HasTextureCoords(0) ? mesh_ai->mNumVertices : 0);
    for (uint32_t i = 0; i < mesh_ai->mNumVertices; ++i) {
        positions.emplace_back(to_glm(mesh_ai->mVertices[i]));
        if (mesh_ai->HasNormals())
            normals.emplace_back(to_glm(mesh_ai->mNormals[i]));
        if (mesh_ai->HasTextureCoords(0))
            texcoords.emplace_back(glm::vec2(to_glm(mesh_ai->mTextureCoords[0][i])));
    }

    // extract faces
    std::vector<uint32_t> indices;
    indices.reserve(mesh_ai->mNumFaces*3);
    for (uint32_t i = 0; i < mesh_ai->mNumFaces; ++i) {
        const aiFace &face = mesh_ai->mFaces[i];
        if (face.mNumIndices == 3) {
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        } else
            std::cout << "WARN: Mesh: skipping non-triangle face!" << std::endl;
    }

    // build GL data
    add_vertex_buffer(GL_FLOAT, 3, positions.size(), positions.data());
    if (not normals.empty())
        add_vertex_buffer(GL_FLOAT, 3, normals.size(), normals.data());
    if (not texcoords.empty())
        add_vertex_buffer(GL_FLOAT, 2, texcoords.size(), texcoords.data());
    add_index_buffer(indices.size(), indices.data());
}

Mesh::Mesh(const std::string& name, const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
            const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texcoords) : Mesh(name) {
    // build GL data
    add_vertex_buffer(GL_FLOAT, 3, positions.size(), positions.data());
    if (not normals.empty())
        add_vertex_buffer(GL_FLOAT, 3, normals.size(), normals.data());
    if (not texcoords.empty())
        add_vertex_buffer(GL_FLOAT, 2, texcoords.size(), texcoords.data());
    add_index_buffer(indices.size(), indices.data());
}

Mesh::~Mesh() {
    if (ibo)
        glDeleteBuffers(1, &ibo);
    glDeleteBuffers(vbo_ids.size(), vbo_ids.data());
    glDeleteVertexArrays(1, &vao);
}

uint32_t Mesh::add_vertex_buffer(GLenum type, uint32_t element_dim, uint32_t num_vertices, const void* data, GLenum hint) {
    if (this->num_vertices && this->num_vertices != num_vertices)
        throw std::runtime_error("Mesh " + name + ": vertex buffer size mismatch!");
    this->num_vertices = num_vertices;
    const uint32_t buf_id = vbo_ids.size();
    vbo_ids.push_back(0);
    vbo_types.push_back(type);
    vbo_dims.push_back(element_dim);
    // setup vbo
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_ids[buf_id]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[buf_id]);
    const size_t stride = type_to_bytes(type) * element_dim;
    glBufferData(GL_ARRAY_BUFFER, stride * num_vertices, data, hint);
    // setup vertex attributes
    glEnableVertexAttribArray(buf_id);
    if (type == GL_BYTE || type == GL_UNSIGNED_BYTE ||
            type == GL_SHORT || type == GL_UNSIGNED_SHORT ||
            type == GL_INT || type == GL_UNSIGNED_INT)
        glVertexAttribIPointer(buf_id, element_dim, type, 0, 0);
    else if (type == GL_DOUBLE)
        glVertexAttribLPointer(buf_id, element_dim, type, 0, 0);
    else
        glVertexAttribPointer(buf_id, element_dim, type, GL_FALSE, 0, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buf_id;
}

void Mesh::add_index_buffer(uint32_t num_indices, const uint32_t* data, GLenum hint) {
    if (!ibo)
        glGenBuffers(1, &ibo);
    this->num_indices = num_indices;
    // setup ibo
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * num_indices, data, hint);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::update_vertex_buffer(uint32_t buf_id, const void* data) {
    if (buf_id > vbo_ids.size())
        throw std::runtime_error("Mesh " + name + ": buffer id out of range!");
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[buf_id]);
    const size_t stride = type_to_bytes(vbo_types[buf_id]) * vbo_dims[buf_id];
    glBufferSubData(GL_ARRAY_BUFFER, 0, stride * num_vertices, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::set_primitive_type(GLenum primitive_type) { this->primitive_type = primitive_type; }

void Mesh::bind() const { glBindVertexArray(vao); }

void Mesh::unbind() const { glBindVertexArray(0); }

void Mesh::draw() const {
    if (ibo)
        glDrawElements(primitive_type, num_indices, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(primitive_type, 0, num_vertices);
}

// ------------------------------------------
// Mesh class

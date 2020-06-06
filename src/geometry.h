#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>
#include "named_handle.h"

// ------------------------------------------
// Geometry

class GeometryImpl;
using Geometry = NamedHandle<GeometryImpl>;

// ------------------------------------------
// GeometryImpl

class GeometryImpl {
public:
    GeometryImpl();
    GeometryImpl(const aiMesh* mesh_ai);
    GeometryImpl(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
            const std::vector<glm::vec3>& normals = std::vector<glm::vec3>(), const std::vector<glm::vec2>& texcoords = std::vector<glm::vec2>());
    virtual ~GeometryImpl();

    explicit inline operator bool() const  { return positions.size() > 0 && indices.size() > 0; }

    void normalize(); // map positions into [-1, 1]
    void recompute_aabb(); // recompute bb_min and bb_max

    void add(const aiMesh* mesh_ai);
    void add(const GeometryImpl& other);
    void add(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
            const std::vector<glm::vec3>& normals = std::vector<glm::vec3>(), const std::vector<glm::vec2>& texcoords = std::vector<glm::vec2>());
    void clear();

    inline bool has_normals() const { return !normals.empty(); }
    inline bool has_texcoords() const { return !texcoords.empty(); }

    // data
    glm::vec3 bb_min, bb_max;
    std::vector<glm::vec3> positions;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
};

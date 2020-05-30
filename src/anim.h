#pragma once

#include <string>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include "named_map.h"
#include "camera.h"

// TODO payload

struct AnimNode {
    glm::vec3 pos;
    glm::quat rot;
};

class Animation : public NamedMap<Animation> {
public:
    Animation(const std::string& name);
    virtual ~Animation();

    void add_node();
    void add_node(const Camera& cam);
    void add_node(const AnimNode& node);
    void insert_node(size_t i, const AnimNode& node);
    void clear();
    size_t size() const;

    void update(float dt_ms);

    void play();
    void pause();
    void stop();
    void reset();

    // data
    std::vector<AnimNode> nodes;
    float t;
    float ms_between_nodes;
    bool running;
};

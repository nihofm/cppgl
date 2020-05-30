#include "anim.h"

Animation::Animation(const std::string& name) : NamedMap(name), t(0), ms_between_nodes(1000), running(false) {}

Animation::~Animation() {}

void Animation::add_node() {
    add_node(*Camera::current());
}

void Animation::add_node(const Camera& cam) {
    AnimNode node;
    cam.store(node.pos, node.rot);
    add_node(node);
}

void Animation::add_node(const AnimNode& node) {
    nodes.push_back(node);
}

void Animation::insert_node(size_t i, const AnimNode& node) {
    if (i < nodes.size())
        nodes[i] = node;
}

void Animation::clear() {
    nodes.clear();
}

size_t Animation::size() const {
    return nodes.size();
}

void Animation::update(float dt_ms) {
    if (running) {
        t = std::min(t + dt_ms, nodes.size() * ms_between_nodes);
        const size_t at = floor(t / ms_between_nodes);
        const AnimNode& lower = nodes[std::min(at, nodes.size() - 1)];
        const AnimNode& upper = nodes[std::min(at + 1, nodes.size() - 1)];
        // slerp
        const glm::vec3& pos = glm::mix(lower.pos, upper.pos, t / ms_between_nodes - at);
        const glm::quat& rot = glm::slerp(lower.rot, upper.rot, t / ms_between_nodes - at);
        Camera::current()->load(pos, rot);
    }
}

void Animation::play() {
    t = 0;
    running = true;
}

void Animation::pause() {
    running = false;
}

void Animation::stop() {
    t = 0;
    running = false;
}

void Animation::reset() {
    t = 0;
}

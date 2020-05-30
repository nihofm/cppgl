#include "anim.h"
#include "camera.h"

Animation::Animation(const std::string& name) : NamedMap(name), time(0), ms_between_nodes(1000), running(false) {}

Animation::~Animation() {}

void Animation::update(float dt_ms) {
    if (running && time < camera_path.size() && !camera_path.empty()) {
        // eval
        const size_t at = glm::floor(time);
        const auto& lower = camera_path[std::min(at, camera_path.size()) % camera_path.size()];
        const auto& upper = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()];
        // slerp
        const glm::vec3& pos = glm::mix(lower.first, upper.first, time - at); // TODO spline
        const glm::quat& rot = glm::slerp(lower.second, upper.second, time - at);
        Camera::current()->load(pos, rot);
        // advance
        time = glm::min(time + dt_ms / ms_between_nodes, float(camera_path.size()));
    } else
        running = false;
}

void Animation::clear() {
    camera_path.clear();
    data_int.clear();
    data_float.clear();
    data_vec2.clear();
    data_vec3.clear();
    data_vec4.clear();
}

size_t Animation::length() const {
    return camera_path.size();
}

void Animation::play() {
    time = 0;
    running = true;
}

void Animation::pause() {
    running = false;
}

void Animation::toggle_pause() {
    running = !running;
}

void Animation::stop() {
    time = 0;
    running = false;
}

void Animation::reset() {
    time = 0;
}

size_t Animation::push_node(const glm::vec3& pos, const glm::quat& rot) {
    const size_t i = camera_path.size();
    camera_path.push_back(std::make_pair(pos, rot));
    return i;
}

void Animation::put_node(size_t i, const glm::vec3& pos, const glm::quat& rot) {
    if (i < camera_path.size())
        camera_path[i] = std::make_pair(pos, rot);
}

void Animation::put_data(const std::string& name, size_t i, int val) {
    if (data_int[name].size() <= i)
        data_int[name].resize(i + 1);
    data_int[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, float val) {
    if (data_float[name].size() <= i)
        data_float[name].resize(i + 1);
    data_float[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec2& val) {
    if (data_vec2[name].size() <= i)
        data_vec2[name].resize(i + 1);
    data_vec2[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec3& val) {
    if (data_vec3[name].size() <= i)
        data_vec3[name].resize(i + 1);
    data_vec3[name][i] = val;
}

void Animation::put_data(const std::string& name, size_t i, const glm::vec4& val) {
    if (data_vec4[name].size() <= i)
        data_vec4[name].resize(i + 1);
    data_vec4[name][i] = val;
}

glm::vec3 Animation::eval_pos() const {
    const size_t at = glm::floor(time);
    const auto& lower = camera_path[std::min(at, camera_path.size()) % camera_path.size()];
    const auto& upper = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()];
    return glm::mix(lower.first, upper.first, time - at);
}

glm::quat Animation::eval_rot() const {
    const size_t at = glm::floor(time);
    const auto& lower = camera_path[std::min(at, camera_path.size()) % camera_path.size()];
    const auto& upper = camera_path[std::min(at + 1, camera_path.size()) % camera_path.size()];
    return glm::slerp(lower.second, upper.second, time - at);
}

int Animation::eval_int(const std::string& name) const {
    const size_t i = std::min(size_t(glm::floor(time)), data_int.at(name).size() - 1);
    return data_int.at(name).at(i); // no lerp necessary
}

// lerp helper
template <typename T> inline T lerp_data(const std::vector<T>& data, float time) {
    const size_t at = glm::floor(time);
    const auto& lower = data[std::min(at, data.size()) % data.size()];
    const auto& upper = data[std::min(at + 1, data.size()) % data.size()];
    return glm::mix(lower, upper, time - at);
}

float Animation::eval_float(const std::string& name) const {
    return lerp_data<float>(data_float.at(name), time);
}

glm::vec2 Animation::eval_vec2(const std::string& name) const {
    return lerp_data<glm::vec2>(data_vec2.at(name), time);
}

glm::vec3 Animation::eval_vec3(const std::string& name) const {
    return lerp_data<glm::vec3>(data_vec3.at(name), time);
}

glm::vec4 Animation::eval_vec4(const std::string& name) const {
    return lerp_data<glm::vec4>(data_vec4.at(name), time);
}

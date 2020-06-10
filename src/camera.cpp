#include "camera.h"
#include "context.h"
#include "imgui/imgui.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

static CameraPtr current_cam;

CameraPtr current_camera() {
    static CameraPtr default_cam("default");
    return current_cam ? current_cam : default_cam;
}

void make_camera_current(const CameraPtr& cam) {
    current_cam = cam;
}

// ----------------------------------------------------
// CameraImpl

float CameraImpl::default_camera_movement_speed = 0.005;

CameraImpl::CameraImpl() : pos(0, 0, 0), dir(1, 0, 0), up(0, 1, 0), fov_degree(70),
    near(0.01), far(1000), left(-100), right(100), bottom(-100), top(100),
    perspective(true), fix_up_vector(true) {
    update();
}

CameraImpl::~CameraImpl() {
    // TODO debug segfault on reset
    // if (current_camera().ptr.get() == this) // reset
        // current_cam = Camera();
}


void CameraImpl::update() {
    dir = glm::normalize(dir);
    up = glm::normalize(up);
    view = glm::lookAt(pos, pos + dir, up);
    view_normal = glm::transpose(glm::inverse(view));
    proj = perspective ? glm::perspective(fov_degree * float(M_PI / 180), aspect_ratio(), near, far) : glm::ortho(left, right, bottom, top, near, far);
}

void CameraImpl::forward(float by) { pos += by * dir; }
void CameraImpl::backward(float by) { pos -= by * dir; }
void CameraImpl::leftward(float by) { pos -= by * glm::cross(dir, up); }
void CameraImpl::rightward(float by) { pos += by * glm::cross(dir, up); }
void CameraImpl::upward(float by) { pos += by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }
void CameraImpl::downward(float by) { pos -= by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }

void CameraImpl::yaw(float angle) { dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, up)); }
void CameraImpl::pitch(float angle) {
    dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, glm::normalize(glm::cross(dir, up))));
    if (not fix_up_vector) up = glm::normalize(glm::cross(glm::cross(dir, up), dir));
}
void CameraImpl::roll(float angle) { up = glm::normalize(glm::rotate(up, angle * float(M_PI) / 180.f, dir)); }

void CameraImpl::store(glm::vec3& pos, glm::quat& rot) const {
    pos = this->pos;
    rot = glm::quat_cast(view);
}

void CameraImpl::load(const glm::vec3& pos, const glm::quat& rot) {
    this->pos = pos;
    this->view = glm::mat4_cast(rot);
    this->dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    this->up = glm::vec3(view[0][1], view[1][1], view[2][1]);
}

float CameraImpl::aspect_ratio() {
    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT, xywh);
    return xywh[2] / (float)xywh[3];
}

bool CameraImpl::default_input_handler(double dt_ms) {
    bool moved = false;
    if (not ImGui::GetIO().WantCaptureKeyboard) {
        // keyboard
        if (Context::key_pressed(GLFW_KEY_W)) {
            current_camera()->forward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_S)) {
            current_camera()->backward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_A)) {
            current_camera()->leftward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_D)) {
            current_camera()->rightward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_R)) {
            current_camera()->upward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_F)) {
            current_camera()->downward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_Q)) {
            current_camera()->roll(dt_ms * -0.1);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_E)) {
            current_camera()->roll(dt_ms * 0.1);
            moved = true;
        }
    }
    // mouse
    static float rot_speed = 0.05;
    static glm::vec2 last_pos(-1);
    const glm::vec2 curr_pos = Context::mouse_pos();
    if (last_pos == glm::vec2(-1)) last_pos = curr_pos;
    const glm::vec2 diff = last_pos - curr_pos;
    if (not ImGui::GetIO().WantCaptureMouse && Context::mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (glm::length(diff) > 0.01) {
            current_camera()->pitch(diff.y * rot_speed);
            current_camera()->yaw(diff.x * rot_speed);
            moved = true;
        }
    }
    last_pos = curr_pos;
    return moved;
}

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

static Camera* current_camera = 0;
float Camera::default_camera_movement_speed = 0.005;

Camera::Camera(const std::string& name) : NamedMap(name), pos(0, 0, 0), dir(1, 0, 0), up(0, 1, 0),
    fov_degree(70), near(0.01), far(1000), left(-100), right(100), bottom(-100), top(100),
    perspective(true), fix_up_vector(true) {
    update();
}

Camera::~Camera() {
    if (current_camera == this) // reset current ptr
        current_camera = 0;
}

Camera* Camera::current() {
    static Camera default_cam("default");
    return current_camera ? current_camera : &default_cam;
}

void Camera::make_current() {
    current_camera = this;
}

void Camera::update() {
    dir = glm::normalize(dir);
    up = glm::normalize(up);
    view = glm::lookAt(pos, pos + dir, up);
    view_normal = glm::transpose(glm::inverse(view));
    proj = perspective ? glm::perspective(fov_degree * float(M_PI / 180), aspect_ratio(), near, far) : glm::ortho(left, right, bottom, top, near, far);
}

void Camera::forward(float by) { pos += by * dir; }
void Camera::backward(float by) { pos -= by * dir; }
void Camera::leftward(float by) { pos -= by * glm::cross(dir, up); }
void Camera::rightward(float by) { pos += by * glm::cross(dir, up); }
void Camera::upward(float by) { pos += by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }
void Camera::downward(float by) { pos -= by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }

void Camera::yaw(float angle) { dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, up)); }
void Camera::pitch(float angle) {
    dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, glm::normalize(glm::cross(dir, up))));
    if (not fix_up_vector) up = glm::normalize(glm::cross(glm::cross(dir, up), dir));
}
void Camera::roll(float angle) { up = glm::normalize(glm::rotate(up, angle * float(M_PI) / 180.f, dir)); }

void Camera::store(glm::vec3& pos, glm::quat& rot) const {
    pos = this->pos;
    rot = glm::quat_cast(view);
}

void Camera::load(glm::vec3& pos, glm::quat& rot) {
    this->pos = pos;
    this->view = glm::mat4_cast(rot);
    this->dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    this->up = glm::vec3(view[0][1], view[1][1], view[2][1]);
}

float Camera::aspect_ratio() {
    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT, xywh);
    return xywh[2] / (float)xywh[3];
}

bool Camera::default_input_handler(double dt_ms) {
    bool moved = false;
    if (not ImGui::GetIO().WantCaptureKeyboard) {
        // keyboard
        if (Context::key_pressed(GLFW_KEY_W)) {
            current()->forward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_S)) {
            current()->backward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_A)) {
            current()->leftward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_D)) {
            current()->rightward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_R)) {
            current()->upward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_F)) {
            current()->downward(dt_ms * default_camera_movement_speed);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_Q)) {
            Camera::current()->roll(dt_ms * -0.1);
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_E)) {
            Camera::current()->roll(dt_ms * 0.1);
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
            current()->pitch(diff.y * rot_speed);
            current()->yaw(diff.x * rot_speed);
            moved = true;
        }
    }
    last_pos = curr_pos;
    return moved;
}

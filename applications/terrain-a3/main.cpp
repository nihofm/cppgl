#include <iostream>

#include <cppgl/cppgl.h>
#include <imgui/imgui.h>

#include "ufo.h"

using namespace std;

// ---------------------------------------
// globals

std::shared_ptr<Ufo> the_ufo;

// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) Camera::find("default")->make_current();
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) Camera::find("ufocam")->make_current();
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
        if (glIsEnabled(GL_CULL_FACE))
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
    }
    if (Camera::current()->name != "ufocam") return;
    // HINT: https://www.glfw.org/docs/latest/input_guide.html
    if (action == GLFW_REPEAT) return;
    if (key == GLFW_KEY_W) {
    }
    if (key == GLFW_KEY_S) {
    }
    if (key == GLFW_KEY_A) {
    }
    if (key == GLFW_KEY_D) {
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    }
}

void resize_callback(int w, int h) {
    Context::resize(w, h);
}

// ---------------------------------------
// main

int main(int argc, char** argv) {
    // init context and set parameters
    ContextParameters params;
    params.title = "terrain";
    params.font_ttf_filename = concat(EXECUTABLE_DIR, "render-data/fonts/DroidSansMono.ttf");
    params.font_size_pixels = 15;
    Context::init(params);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_resize_callback(resize_callback);

    // EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
    Shader::base_path = EXECUTABLE_DIR;
    Texture2D::base_path = EXECUTABLE_DIR;
    MeshLoader::base_path = EXECUTABLE_DIR;

    // init camera
    auto ufocam = make_camera("ufocam");
    //ufocam->make_current();

    // init stuff
    the_ufo = std::make_shared<Ufo>();
    glClearColor(1, 1, 1, 1);
    glDisable(GL_CULL_FACE); // disable backface culling per default

    while (Context::running()) {
        // input and update
        if (Camera::current()->name != "ufocam")
            Camera::default_input_handler(Context::frame_time());
        Camera::current()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) Shader::reload();

        // render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        the_ufo->draw();

        // finish frame
        Context::swap_buffers();
    }

    return 0;
}

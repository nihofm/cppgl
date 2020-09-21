#include <iostream>

#include <cppgl/cppgl.h>
#include <imgui/imgui.h>

#include "osm.h"
#include "parse.h"
#include "rendering.h"

using namespace std;

// ---------------------------------------
// globals

// TODO additional data structures/meshes for map?

// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) Camera::find("default")->make_current();
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) Camera::find("mapcam")->make_current();
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
    if (Camera::current()->name != "mapcam") return;
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
    params.title = "pacmap";
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
    auto mapcam = make_camera("mapcam");
    mapcam->pos = glm::vec3(0, 5, 0);
    mapcam->dir = glm::vec3(0, -1, 0);
    mapcam->up = glm::vec3(0, 0, 1);
    //mapcam->make_current();

    // init stuff
    glClearColor(1, 1, 1, 1);
    glDisable(GL_CULL_FACE); // disable backface culling per default
    // init map
    setup_map("render-data/map/map2.osm");
    osm::make_meshes();
    // init shader
    auto pos_shader = make_shader("pos", "shader/pos.vs", "shader/pos.fs");

    while (Context::running()) {
        // input and update
        if (Camera::current()->name != "mapcam")
            Camera::default_input_handler(Context::frame_time());
        Camera::current()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) Shader::reload();

        // render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        osm::draw_meshes(pos_shader);

        // finish frame
        Context::swap_buffers();
    }

    return 0;
}

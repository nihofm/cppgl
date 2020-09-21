#include <iostream>

#include <cppgl/cppgl.h>
#include <imgui/imgui.h>

#include "cmdline.h"
#include "particles.h"
#include "rendering.h"
#include "static-view-elements.h"
#include "dynamic-view-elements.h"
#include "clientside-networking.h"

using namespace std;

// ---------------------------------------
// globals

bool game_is_running = false;
std::shared_ptr<Board> the_board;
std::shared_ptr<Floor> the_floor;
std::shared_ptr<Skybox> the_skybox;
std::shared_ptr<Fog> the_fog;
std::vector<std::shared_ptr<Player>> players;
std::shared_ptr<Particles> particles, particles_small;
std::shared_ptr<Framebuffer> gbuffer;
int player_id = -1;
boost::asio::ip::tcp::socket* server_connection = 0;
client_message_reader* reader = 0;
bool gbuffer_debug = false;

// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (!reader || !reader->prologue_over()) return;
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) Camera::find("default")->make_current();
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) Camera::find("playercam")->make_current();
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) gbuffer_debug = !gbuffer_debug;
    if (Camera::current()->name != "playercam") return;
    // HINT: https://www.glfw.org/docs/latest/input_guide.html
    if (action == GLFW_REPEAT) return;
    if (key == GLFW_KEY_W) {
        msg::key_updown kd = make_message<msg::key_updown>();
        kd.k = msg::key_code::up;
        kd.down = action == GLFW_PRESS ? 1 : 0;
        send_message(kd);
    }
    if (key == GLFW_KEY_S) {
        msg::key_updown kd = make_message<msg::key_updown>();
        kd.k = msg::key_code::down;
        kd.down = action == GLFW_PRESS ? 1 : 0;
        send_message(kd);
    }
    if (key == GLFW_KEY_A) {
        msg::key_updown kd = make_message<msg::key_updown>();
        kd.k = msg::key_code::left;
        kd.down = action == GLFW_PRESS ? 1 : 0;
        send_message(kd);
    }
    if (key == GLFW_KEY_D) {
        msg::key_updown kd = make_message<msg::key_updown>();
        kd.k = msg::key_code::right;
        kd.down = action == GLFW_PRESS ? 1 : 0;
        send_message(kd);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        msg::key_drop kd = make_message<msg::key_drop>();
        send_message(kd);
    }
}

void resize_callback(int w, int h) {
    if (gbuffer) gbuffer->resize(w, h);
}

// ---------------------------------------
// main

int main(int argc, char** argv) {
    parse_cmdline(argc, argv);

    // init context and set parameters
    ContextParameters params;
    params.title = "bbm";
    params.font_ttf_filename = concat(EXECUTABLE_DIR, "render-data/fonts/DroidSansMono.ttf");
    params.font_size_pixels = 15;
    Context::init(params);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_resize_callback(resize_callback);

    // EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
    Shader::base_path = EXECUTABLE_DIR;
    Texture2D::base_path = EXECUTABLE_DIR;
    MeshLoader::base_path = EXECUTABLE_DIR;

    auto playercam = make_camera("playercam");
    playercam->far = 250;
    playercam->make_current();

    const glm::ivec2 res = Context::resolution();
    gbuffer = make_framebuffer("gbuffer", res.x, res.y);
    gbuffer->attach_depthbuffer(make_texture("gbuf_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
    gbuffer->attach_colorbuffer(make_texture("gbuf_diff", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(make_texture("gbuf_pos", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(make_texture("gbuf_norm", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->check();

    auto game_over_tex = make_texture("game-over", "render-data/images/gameover.png");

    init_static_prototypes();
    init_dynamic_prototypes();
    particles = std::make_shared<Particles>(2000, render_settings::particle_size);
    particles_small = std::make_shared<Particles>(3000, render_settings::particle_size * 0.1);

    networking_prologue();

    TimerQuery input_timer("input");
    TimerQuery update_timer("update");
    TimerQueryGL render_timer("render");
    TimerQueryGL postprocess_timer("postprocess");

    while (Context::running() && game_is_running) {
        // input handling
        input_timer.start();
        if (Camera::current()->name != "playercam")
            Camera::default_input_handler(Context::frame_time());
        reader->read_and_handle();
        Camera::current()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) Shader::reload();
        input_timer.end();

        // update
        update_timer.start();
        for (auto& player : players)
            player->update();
        the_board->update();
        particles->update();
        particles_small->update();
        update_timer.end();

        // render
        render_timer.start();
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto& player : players)
            player->draw();
        the_board->draw();
        the_floor->draw();
        gbuffer->unbind();
        render_timer.end();

        // postprocess
        postprocess_timer.start();
        if (gbuffer_debug)
            deferred_debug_pass(gbuffer);
        else {
            deferred_shading_pass(gbuffer);
            the_skybox->draw();
            particles->draw();
            particles_small->draw();
            the_fog->draw();
        }
        draw_gui();
        postprocess_timer.end();

        // finish frame
        Context::swap_buffers();
    }

    Timer game_over_timer;
    while (Context::running() && game_over_timer.look() < 1337) {
        blit(game_over_tex);
        Context::swap_buffers();
    }

    return 0;
}

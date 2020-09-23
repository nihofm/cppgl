#include <iostream>

#include <cppgl.h>

#include "cmdline.h"
#include "particles.h"
#include "rendering.h"
#include "static-view-elements.h"
#include "dynamic-view-elements.h"
#include "clientside-networking.h"
#undef far
#undef near

using namespace std;

// ---------------------------------------
// globals

bool game_is_running = false;
std::shared_ptr<Board> the_board;
std::shared_ptr<Floor> the_floor;
std::shared_ptr<Skybox> the_skybox;
std::vector<std::shared_ptr<Player>> players;
std::shared_ptr<Particles> particles, particles_small;
Framebuffer fbo;
int player_id = -1;
boost::asio::ip::tcp::socket* server_connection = 0;
client_message_reader* reader = 0;
boost::asio::io_service io_service;
// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (!reader || !reader->prologue_over()) return;
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) make_camera_current(Camera::find("default"));
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) make_camera_current(Camera::find("playercam"));
    if (current_camera()->name != "playercam") return;

    // HINT: https://www.glfw.org/docs/latest/input_guide.html

	// HINT key handling missing
#ifndef A1_1
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
#endif // A1_1 key handlilng

#ifndef A1_5
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        msg::key_drop kd = make_message<msg::key_drop>();
        send_message(kd);
    }
#endif

}

void resize_callback(int w, int h) {
    if (fbo) fbo->resize(w, h);
}

// ---------------------------------------
// main

int main(int argc, char** argv) {
    if(!parse_cmdline(argc, argv)) return 0;

    // init context and set parameters
    ContextParameters params;
    params.title = "bbm";
    params.font_ttf_filename = EXECUTABLE_DIR + std::string("render-data/fonts/DroidSansMono.ttf");
    params.font_size_pixels = 15;
    Context::init(params);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_resize_callback(resize_callback);

    // EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
    std::filesystem::current_path(EXECUTABLE_DIR);

    auto playercam = Camera("playercam");
    playercam->far = 250;
    make_camera_current(playercam);

    const glm::ivec2 res = Context::resolution();
    fbo = Framebuffer("target_fbo", res.x, res.y);
    fbo->attach_depthbuffer(Texture2D("fbo_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
    fbo->attach_colorbuffer(Texture2D("fbo_color", res.x, res.y, GL_RGBA32F, GL_RGBA, GL_FLOAT));
    fbo->check();

    auto game_over_tex = Texture2D("game-over", "render-data/images/gameover.png");

    init_static_prototypes();
    init_dynamic_prototypes();
    particles = std::make_shared<Particles>(2000, render_settings::particle_size);
    particles_small = std::make_shared<Particles>(3000, render_settings::particle_size * 0.1);

    networking_prologue();

    TimerQuery input_timer("input");
    TimerQuery update_timer("update");
    TimerQueryGL render_timer("render");

    while (Context::running() && game_is_running) {
        // input handling
        input_timer.begin();
        if (current_camera()->name != "playercam")
            CameraImpl::default_input_handler(Context::frame_time());

        reader->read_and_handle();
        current_camera()->update();
        reload_modified_shaders();
        input_timer.end();

        // update
        update_timer.begin();
        for (auto& player : players)
            player->update();
        the_board->update();
        particles->update();
        particles_small->update();
        update_timer.end();

        // render
        render_timer.begin();
        fbo->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto& player : players)
            player->draw();
        the_board->draw();
        the_floor->draw();
        fbo->unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        blit_depth(fbo->depth_texture, fbo->color_textures[0]);
        the_skybox->draw();
        particles->draw();
        particles_small->draw();
        draw_gui();
        render_timer.end();

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

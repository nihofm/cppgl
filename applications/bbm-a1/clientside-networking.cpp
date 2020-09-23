#include "clientside-networking.h"

#include <iostream>
#include <memory>

#include <cppgl.h>

#include "static-view-elements.h"
#include "dynamic-view-elements.h"
#include "cmdline.h"

using namespace std;

// ---------------------------------------
// globals

extern bool game_is_running;
extern std::shared_ptr<Board> the_board;
extern std::shared_ptr<Floor> the_floor;
extern std::shared_ptr<Skybox> the_skybox;
extern std::vector<std::shared_ptr<Player>> players;
extern int player_id;
extern boost::asio::ip::tcp::socket* server_connection;
extern client_message_reader* reader;
extern boost::asio::io_service io_service;


// ---------------------------------------

void send_message(msg::message& msg) {
	try {
		boost::asio::write(*server_connection, boost::asio::buffer(&msg, msg.message_size), boost::asio::transfer_all());
	} catch (boost::system::system_error& err) {
		cerr << "Error sending message to the server: " << err.what() << endl << "Exiting." << endl;
		exit(-1);
	}
}

void draw_start_screen(const std::string& add = "", bool error = false) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::ivec2 screen_size = Context::resolution();
    ImGui::SetNextWindowPos(ImVec2(200, screen_size.y - 200));
    ImGui::SetNextWindowSize(ImVec2(screen_size.x - 200, 200));
    if (ImGui::Begin("Loading", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground)) {
        ImGui::SetWindowFontScale(3.f);
        ImGui::TextColored(error ? ImVec4(1,0,0,1) : ImVec4(0,1,0,1), "Loading%s", add.c_str());
    }
    ImGui::End();

    Context::swap_buffers();
}

void networking_prologue() {
	using boost::asio::ip::tcp;
	
    draw_start_screen();


	try {
		cout << "connecting..." << endl;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(cmdline.server_host, "2214");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;
		
        draw_start_screen(".");

		server_connection = new tcp::socket(io_service);
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end) {
			server_connection->close();
			server_connection->connect(*endpoint_iterator++, error);
		}
		if (error)
			throw boost::system::system_error(error);

		cout << "connected." << endl;
		
        draw_start_screen("..");

		msg::player_name pn = make_message<msg::player_name>();
		strncpy(pn.name, cmdline.player_name.c_str(), msg::player_name::max_string_length);
		pn.name[msg::player_name::max_string_length-1] = '\0';
		pn.id = 0;
		send_message(pn);
		
        draw_start_screen("...");

		reader = new client_message_reader(server_connection);


		while (!reader->eof()) {

            draw_start_screen("....");

			reader->read_and_handle();
			if (reader->prologue_over()) {
				Timer timer;
				while (timer.look() < 500) { draw_start_screen("..... done."); }
				break;
			}
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		string text = " (" + string(e.what()) + ")";
		Timer timer;
		while (timer.look() < 1337) { draw_start_screen("....." + text, true); }
		exit(EXIT_FAILURE);
	}
}

client_message_reader::client_message_reader(message_reader::socket *sock) : message_reader(sock) {
	setup_done = false;
}

bool client_message_reader::prologue_over() {
	return setup_done;
}

void client_message_reader::handle_message(msg::connected *m) {
	cout << "you are connected. your id is " << (int)m->your_id << "/" << (int)m->players << endl;
	player_id = m->your_id;
    players.resize(m->players);
	for (unsigned int i = 0; i < players.size(); ++i)
		players[i] = std::make_shared<Player>(("player-" + std::to_string(i)).c_str(), i);
}

void client_message_reader::handle_message(msg::initial_player_data_position* m) {
	cout << "the initial player position for player " << (int)m->player << " is " << (int)m->x << ", " << (int)m->y << endl;
	players[m->player]->force_position(m->x, m->y);
    if (m->player == player_id) {
        const glm::vec2& cam_offset = render_settings::character_camera_offset;
        auto cam = Camera::find("playercam");
        cam->pos = glm::vec3(m->x * render_settings::tile_size, cam_offset.y, m->y * render_settings::tile_size + cam_offset.x);
        cam->dir = glm::normalize(glm::vec3(m->x * render_settings::tile_size, 0, m->y * render_settings::tile_size) - cam->pos);
        cam->up = glm::vec3(0, 1, 0);
        cam->update();
    }
}

void client_message_reader::handle_message(msg::board_info* m) {
	cout << "the board is " << (int)m->w << " x " << (int)m->h << " with " << (int)m->boxes << " boxes." << endl;
	int tiles_x = m->w;
	int tiles_y = m->h;
	the_board = std::make_shared<Board>(tiles_x, tiles_y);
    the_floor = std::make_shared<Floor>(tiles_x, tiles_y);
    the_skybox = std::make_shared<Skybox>(tiles_x, tiles_y, 2 * max(tiles_x, tiles_y));
}

void client_message_reader::handle_message(msg::spawn_box* m) {
    //cout << "a box (type " << (int)m->box_type << ") spanws at " << (int)m->x << ", " << (int)m->y << endl;
	the_board->add_box(m->x, m->y, m->box_type);
}

void client_message_reader::handle_message(msg::start_game *m) {
	(void)m;
	cout << "game on!" << endl;
	setup_done = true;
    game_is_running = true;
}

void client_message_reader::handle_message(msg::force_player_position* m) {
	cout << "forced player pos to " << (int)m->x << "," << (int)m->y << endl;
	// Setting health to 100 and enforcing the position indicates a respawn
	if (players[m->player]->respawning) {
		players[m->player]->respawning = false;
        // recenter relative cam
        if (m->player == player_id) {
            const glm::vec2& cam_offset = render_settings::character_camera_offset;
            auto cam = Camera::find("playercam");
            cam->pos = glm::vec3(m->x * render_settings::tile_size, cam_offset.y, m->y * render_settings::tile_size + cam_offset.x);
            cam->dir = glm::normalize(glm::vec3(m->x * render_settings::tile_size, 0, m->y * render_settings::tile_size) - cam->pos);
            cam->up = glm::vec3(0, 1, 0);
            cam->update();
		}
	}
    players[m->player]->force_position(m->x,m->y);
}

void client_message_reader::handle_message(msg::start_move* m) {
	cout << "start moving in direction " << (int)m->dir_x << "," << (int)m->dir_y << " for about " << (int)m->est_duration << " ms" << endl;
	players[m->player]->start_moving(m->dir_x, m->dir_y, m->est_duration);
}

void client_message_reader::handle_message(msg::spawn_bomb* m) {
	cout << "bomb " << (int)m->id << " spawns at " << (int)m->x << ", " << (int)m->y << endl;
	the_board->add_bomb(m->x, m->y, m->id);
}

void client_message_reader::handle_message(msg::bomb_explosion* m) {
	cout << "bomb " << (int)m->id << " explodes" << endl;
	the_board->explosion(m->id, m->dir_codes);
}

void client_message_reader::handle_message(msg::player_health_info* m) {
	cout << "player" << (int)m->id << "'s health is " << (int)m->health << endl;
	players[m->id]->health = m->health;
	// check for respawn
	if (players[m->id]->health == 100)
		players[m->id]->respawning = true;
}

void client_message_reader::handle_message(msg::player_name* m) {
	cout << "player" << (int)m->id << "'s name is " << m->name << endl;
	players[m->id]->name = m->name;
}

void client_message_reader::handle_message(msg::frags_update* m) {
	cout << "player" << (int)m->player << "'s new frag count is " << (int)m->frags << endl;
	players[m->player]->frags = m->frags;
}

void client_message_reader::handle_message(msg::game_over* m) {
	(void)m;
	game_is_running = false;
}

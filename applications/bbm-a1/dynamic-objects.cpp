#include "dynamic-objects.h"
#include "clientside-networking.h"
#include "particles.h"

#include <cppgl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

using namespace std;

// ------------------------------------------------
// globals

extern int player_id;
extern std::shared_ptr<Particles> particles, particles_small;

// ------------------------------------------------
// prototypes

std::vector<Drawelement> Player::prototype;
Material Box::wood_material;
std::vector<Material> Box::stone_materials;
std::vector<Drawelement> Box::prototype_idle;
std::vector<Drawelement> Box::prototype_scatter;
std::vector<Drawelement> Bomb::prototype;

void init_dynamic_prototypes() {
	{ // load player prototype
		auto shader_norm = Shader("bbm: pos+norm", "shader/pos+norm.vs", "shader/pos+norm.fs");
		auto shader_norm_tc = Shader("bbm: pos+norm+tc,modulated", "shader/pos+norm+tc.vs", "shader/pos+norm+tc.fs");
		auto shader_callback = [shader_norm, shader_norm_tc](const Material& mat) {
			return mat->texture_map.empty() ? shader_norm : shader_norm_tc;
		};
		auto meshes = load_meshes_gpu("render-data/bbm/bbm-nolegs.obj", true);
		for (auto m : meshes) {
			Player::prototype.push_back(Drawelement(m->name, shader_callback(m->material), m));
		}
	}
	{ // load bomb prototype
		auto shader_norm = Shader("bomb: pos+norm", "shader/pos+norm.vs", "shader/pos+norm.fs");
		auto shader_norm_tc = Shader("bomb: pos+norm+tc", "shader/pos+norm+tc.vs", "shader/pos+norm+tc.fs");
		auto shader_callback = [shader_norm, shader_norm_tc](const Material& mat) {
			return mat->texture_map.empty() ? shader_norm : shader_norm_tc;
		};
		auto meshes = load_meshes_gpu("render-data/bomb/bomb.obj", true);
		for (auto m : meshes) {
			Bomb::prototype.push_back(Drawelement(m->name, shader_callback(m->material), m));
		}
	}
	{ // load box prototypes
		auto shader = Shader("box-shader", "shader/box.vs", "shader/box.fs");
		auto shader_cb = [shader](const Material&) { return shader; };
		// load idle and scatter box
		{
			auto meshes = load_meshes_gpu("render-data/cube/cube.obj", true);
			for (auto m : meshes) {
				Box::prototype_idle.push_back(Drawelement(m->name, shader_cb(m->material), m));
			}
		}
		{
			auto meshes = load_meshes_gpu("render-data/crate/wooden_crate.obj", true);
			for (auto m : meshes) {
				Box::prototype_scatter.push_back(Drawelement(m->name, shader_cb(m->material), m));
			}
		}
		// load materials
		auto overlay = Texture2D("crate_overlay", "render-data/images/crate_overlay2.png");
		Box::wood_material = Material("box-wood");
		Box::wood_material->add_texture("diffuse", Texture2D("box-wood", "render-data/crate/crate.png"));
		Box::wood_material->add_texture("normalmap", Texture2D("box-wood-normals", "render-data/crate/crate_normals.png"));
		Box::wood_material->add_texture("overlay", overlay);
		auto mat0 = Material("box-wall_0");
		mat0->add_texture("diffuse", Texture2D("box-wall_diff_0", "render-data/images/wall_0.png"));
		mat0->add_texture("normalmap", Texture2D("box-wall_normals_0", "render-data/images/wall_normals_0.png"));
		mat0->add_texture("overlay", overlay);
		Box::stone_materials.push_back(mat0);
		auto mat1 = Material("box-wall_1");
		mat1->add_texture("diffuse", Texture2D("box-wall_diff_1", "render-data/images/wall_1.png"));
		mat1->add_texture("normalmap", Texture2D("box-wall_normals_1", "render-data/images/wall_normals_1.png"));
		mat1->add_texture("overlay", overlay);
		Box::stone_materials.push_back(mat1);
		auto mat2 = Material("box-wall_2");
		mat2->add_texture("diffuse", Texture2D("box-wall_diff_2", "render-data/images/wall_2.png"));
		mat2->add_texture("normalmap", Texture2D("box-wall_normals_2", "render-data/images/wall_normals_2.png"));
		mat2->add_texture("overlay", overlay);
		Box::stone_materials.push_back(mat2);
	}
}

// ------------------------------------------------
// Player

Player::Player(const std::string& name, int id) : moving(false), look_dir(0, 1),
name(name), model(1), model_rot(1), health(100), id(id), respawning(false), frags(0) {
	model[0][0] = render_settings::character_radius;
	model[1][1] = render_settings::character_radius;
	model[2][2] = render_settings::character_radius;
	model[3][1] = render_settings::character_float_h;
	// Start wobble timer
	wobble_timer.begin();
}

float Player::rotation(Dir dir) {
	if (dir == Dir(0, 1))  return 0;
	if (dir == Dir(0, -1)) return float(M_PI);
	if (dir == Dir(1, 0))  return -float(M_PI) / 2.0f;
	if (dir == Dir(-1, 0)) return float(M_PI) / 2.0f;
	return 0;
}

float Player::rotation_angle_between(Dir from, Dir to) {
	float old_rot = rotation(from);
	float new_rot = rotation(to);
	float diff = old_rot - new_rot;
	if (abs(diff) > M_PI)		// if rotation angle > 180, rotate in other direction
		diff -= glm::sign(diff) * 2.f * float(M_PI);
	return diff;
}

void Player::force_position(int x, int y) {
	moving = false;
	glm::vec3 axis(0, 1, 0);
	base_rotation = -rotation(look_dir);
	model_rot = glm::rotate(glm::mat4(1), base_rotation, glm::vec3(0, 1, 0));
	model[3][0] = x * render_settings::tile_size;
	model[3][2] = y * render_settings::tile_size;
	base = glm::vec3(x, 0, y);
}

void Player::start_moving(int dir_x, int dir_y, int est_duration) {
	if (moving) {	// the last movement is not finished: reset base
		base = base + move_dir;
		model[3][0] = base.x * render_settings::tile_size;
		model[3][2] = base.z * render_settings::tile_size;
		base_rotation = -rotation(look_dir);
	}

	move_dir = glm::vec3(dir_x, 0, dir_y);
	move_duration = float(est_duration);
	move_rotation = rotation_angle_between(look_dir, Dir(dir_x, dir_y));

	look_dir.x = dir_x;
	look_dir.y = dir_y;
	moving = true;
	movement_timer.begin();
}

void Player::update() {
	if (health <= 0) return;

	if (moving) {
		const float elapsed = float(movement_timer.look());
		// HINT: Right here we may have skipped something tagged 'smooth player movement'
		// NOTE: Use matrix 'model' for translation/scale and 'model_rot' for rotation
#ifndef A1_2
		const float t = std::min(1.0f, elapsed / move_duration);
		model_rot = glm::rotate(glm::mat4(1), base_rotation + t * move_rotation, glm::vec3(0, 1, 0));
		const glm::vec3 curr = (base + move_dir * t) * render_settings::tile_size;
		model[3][0] = curr.x;
		model[3][2] = curr.z;
#endif // A1_2

		// HINT: Right here we may have skipped something tagged 'smooth camera movement'
		const glm::vec2& cam_offset = render_settings::character_camera_offset;
#ifndef A1_3
		if (id == player_id) {
			auto cam = Camera::find("playercam");
			cam->pos = glm::vec3(curr.x, cam_offset.y, curr.z + cam_offset.x);
			cam->dir = glm::normalize(curr - cam->pos);
			cam->update();
		}
#endif // A1_3
	}

	// HINT: Right here we may have skipped something tagged 'character idle animation'
#ifndef A1_4
	model[3][1] = render_settings::character_float_h + 0.2f * render_settings::character_radius +
		glm::sin(float(player_id) * 10.0f + glm::cos(0.002f * wobble_timer.look() + glm::cos(0.001f * wobble_timer.look())));
#endif // A1_4

	// player particles
	if (particle_timer.look() >= render_settings::particle_emitter_timeslice) {
		particle_timer.begin();
		const float offset = 0.9f * render_settings::character_radius;
		const glm::vec3 pos = glm::vec3(model[3][0], model[3][1] - offset, model[3][2]);
		const glm::vec3 dir = glm::normalize(glm::vec3(random_float(), -3, random_float()));
		particles->add(pos, dir, (rand() % 1000) + 1000);
		// HINT: Right here we may have skipped something tagged 'even more particles'
#ifndef A1_8
		for (int i = 0; i < 3; ++i)
			particles_small->add(pos, dir + glm::vec3(random_float(), random_float(), random_float()), (rand() % 1000) + 1000);
#endif // A1_8
	}
}

void Player::draw() {
	if (health <= 0) return;
	for (auto& elem : prototype) {
		elem->model = model * model_rot;
		elem->bind();
		setup_light(elem->shader);
		elem->draw();
		elem->unbind();
	}
}

// ------------------------------------------------
// Box

Box::Box(unsigned posx, unsigned posy, bool is_stone) : model(1.f), is_stone(is_stone), exploding(false) {
	stone_type = is_stone ? rand() % 3 : 0;
	uv_offset.x = float(rand() % 100) / 100.0f;
	uv_offset.y = float(rand() % 100) / 100.0f;
	model[0][0] = 0.5 * render_settings::tile_size;
	model[1][1] = 0.5 * render_settings::tile_size;
	model[2][2] = 0.5 * render_settings::tile_size;
	model[3][0] = posx * render_settings::tile_size;
	model[3][1] = 0.5 * render_settings::tile_size;
	model[3][2] = posy * render_settings::tile_size;
}

void Box::set_exploding(const glm::vec2& dir) {
	exploding = true;
	// HINT: Right here we may have skipped something tagged 'crate explosion'
#ifndef A1_8_ex
	explo_timer.begin();
	explo_rot_axis.resize(prototype_scatter.size());
	explo_translation.resize(prototype_scatter.size());
	for (unsigned int i = 0; i < prototype_scatter.size(); ++i) {
		explo_rot_axis[i] = glm::normalize(glm::vec3(random_float(), random_float(), random_float()));
		explo_translation[i] = glm::vec3(
			dir.x * 20.f * fabs(random_float()) + 3.f * random_float(),
			5.f + 12.f * fabs(random_float()),
			dir.y * 20.f * fabs(random_float()) + 3.f * random_float());
		// HINT: Right here we may have skipped something tagged 'even more particles'
		for (int p = 0; p < 10; ++p) {
			const glm::vec3 pos = glm::vec3(model[3]);
			const glm::vec3 dir = glm::normalize(explo_translation[i]) * glm::length(explo_translation[i]) * (random_float() + 1) * 0.3f;
			particles_small->add(pos + 0.5f * render_settings::tile_size * glm::vec3(random_float(), random_float(), random_float()),
				dir + glm::vec3(random_float(), random_float(), random_float()), (rand() % 750) + 750);
		}
	}
#endif

}

bool Box::to_destroy() { 
	// HINT: Right here we may have skipped something tagged 'crate explosion'
#ifndef A1_8_ex
	return exploding && explo_timer.look() > render_settings::box_explosion_duration;
#else
	return exploding; 
#endif

}

void Box::draw() {
	if (!exploding) {
		for (auto& elem : prototype_idle) {
			elem->model = model;
			elem->bind();
			// bind wood or stone material
			if (is_stone)
				Box::stone_materials[stone_type]->bind(elem->shader);
			else
				Box::wood_material->bind(elem->shader);
			elem->shader->uniform("uv_offset", uv_offset);
			setup_light(elem->shader);
			elem->draw();
			elem->unbind();
		}
	}
	// HINT: Right here we may have skipped something tagged 'crate explosion'
#ifndef A1_8_ex
	else if (!to_destroy()) {
		for (size_t i = 0; i < prototype_scatter.size(); ++i) {
			// apply explosion translation and rotation
			const float t = powf(explo_timer.look() / render_settings::box_explosion_duration, 0.75);
			glm::mat4 model = glm::translate(model, t * explo_translation[i]);
			model = glm::rotate(model, t * float(5 * M_PI), glm::vec3(explo_rot_axis[i]));

			prototype_scatter[i]->model = model;
			prototype_scatter[i]->bind();
			Box::wood_material->bind(prototype_scatter[i]->shader);
			prototype_scatter[i]->shader->uniform("uv_offset", uv_offset);
			setup_light(prototype_scatter[i]->shader);

			prototype_scatter[i]->draw();
			prototype_scatter[i]->unbind();
		}
	}
#endif
}

// ------------------------------------------------
// Bomb

Bomb::Bomb(int posx, int posy, int id) : x(posx), y(posy), id(id), model(1) {
	// HINT: Right here we may have skipped something tagged 'render bombs'
#ifndef A1_6
	model[0][0] = render_settings::bomb_radius;
	model[1][1] = render_settings::bomb_radius;
	model[2][2] = render_settings::bomb_radius;
	model[3][0] = float(posx) * render_settings::tile_size;
	model[3][1] = render_settings::bomb_radius;
	model[3][2] = float(posy) * render_settings::tile_size;
#endif // A1_6
}

void Bomb::draw() {

	for (auto& elem : prototype) {
		elem->model = model;
		elem->bind();
		setup_light(elem->shader);
		elem->draw();
		elem->unbind();
	}
	// HINT: Right here we may have skipped something tagged 'even more particles'
#ifndef A1_8
	particles_small->add(glm::vec3(model[3]) + glm::vec3(0, render_settings::bomb_radius, 0),
		1.5f * glm::normalize(glm::vec3(random_float(), 2, random_float())), (rand() % 750) + 500);
#endif // A1_8
}

// ------------------------------------------------
// Board

Board::Board(int tiles_x, int tiles_y) : tiles_x(tiles_x), tiles_y(tiles_y), camera_shake(false) {
	// init cells
	cells = std::vector<std::vector<Cell>>(tiles_y);
	for (int i = 0; i < tiles_y; ++i) {
		cells[i] = (std::vector<Cell>(tiles_x));
		for (int j = 0; j < tiles_x; ++j)
			cells[i][j].occupied = 0;
	}
}

void Board::add_box(int x, int y, int type) {
	const bool is_stone = type == msg::box_type::stone;
	auto box = std::make_shared<Box>(x, y, is_stone);
	cells[y][x].occupied = type;
	cells[y][x].box = box;
	if (is_stone)
		stone_boxes.push_back(box);
	else
		crates.push_back(box);
}

void Board::add_bomb(int x, int y, int id) {
	bombs.push_back(std::make_shared<Bomb>(x, y, id));
}

void Board::update() {

	// HINT: Right here we may have skipped something tagged 'box explosion'
#ifndef A1_8_ex
	for (auto it = crates.begin(); it != crates.end(); ) {
		if ((*it)->to_destroy())
			it = crates.erase(it);
		else
			++it;
	}
#endif //A1_8_ex
	// HINT: Right here we may have skipped something tagged 'camera shake'
#ifndef A1_9
	if (camera_shake) {
		// Stop camera shake after camera_shake_duration ms
		if (camera_shake_timer.look() > render_settings::camera_shake_duration)
			camera_shake = false;
		else {
			// Apply the camera shake
			auto cam = Camera::find("playercam");
			const float t = render_settings::camera_shake_duration - camera_shake_timer.look();
			cam->view[3][0] += 0.25f * sin(0.03f * t * sin(0.02f * t));
			cam->view[3][1] += 0.25f * cos(0.03f * t * sin(0.02f * t));
			cam->view[3][2] += 0.25f * cos(0.03f * t * cos(0.02f * t));
		}
	}
#endif // A1_9
}

void Board::draw() {
	for (auto& elem : stone_boxes)
		elem->draw();
	for (auto& elem : crates)
		elem->draw();
	for (auto& elem : bombs)
		elem->draw();
}

void Board::explosion(int bomb_id, unsigned int codes) {
	// Camera wobble on explode
	if (!camera_shake) {
		camera_shake_timer.begin();
		camera_shake = true;
	}

	std::shared_ptr<Bomb> the_bomb;
	for (auto it = bombs.begin(); it != bombs.end(); ++it) {
		if ((*it)->id == bomb_id) {
			the_bomb = *it;
			bombs.erase(it);
			break;
		}
	}

	if (!the_bomb) {
		cerr << "ERROR: The server triggered an explosion for a bomb we don't have recorded!" << endl;
		return;
	}
	// HINT: Right here we may have skipped something tagged 'map update'
#ifndef A1_7
	int x = the_bomb->x, y = the_bomb->y;
	int x_pos = codes & 0x3;	codes >>= 2;
	int x_neg = codes & 0x3;	codes >>= 2;
	int y_pos = codes & 0x3;	codes >>= 2;
	int y_neg = codes & 0x3;
	cout << x_pos << " | " << x_neg << " | " << y_pos << " | " << y_neg << endl;

	struct direction { int x, y, z, len; };
	direction dirs[4] = { {1, 0, 0, x_pos}, {-1, 0, 0, x_neg}, {0, 0, 1, y_pos}, {0, 0, -1, y_neg} };
	glm::vec3 center = glm::vec3(float(x) * render_settings::tile_size, render_settings::tile_size / 2, float(y) * render_settings::tile_size);

	for (int i = 0; i < 4; ++i) {
		for (int j = 1; j <= dirs[i].len; ++j) {
			auto& cell = cells[y + j * dirs[i].z][x + j * dirs[i].x];
			if (!cell.box) continue;
			int type = cell.occupied;
			if (type == msg::box_type::stone)
				cerr << "ERROR: cannot destory a stonebox. this may be a communication error." << endl;
			else if (type == msg::box_type::wood) {
				cell.box->set_exploding(glm::vec2(dirs[i].x, dirs[i].z));
				cell.box = 0;
			}
		}
	}
#endif // A1_7

	// HINT: Right here we may have skipped something tagged 'bomb particles'
#ifndef A1_8
	for (int i = 0; i < 200; ++i) {
		glm::vec3 jitter = random_vec3() * render_settings::tile_size / 7.f;
		int d = 0;
		do d = rand() % 4; while (dirs[d].len == 0);
		glm::vec3 pos; pos = center + jitter;
		glm::vec3 dir = { (float)dirs[d].x, (float)dirs[d].y, (float)dirs[d].z };
		if (dir.x == 0) dir.x = random_float() / 10; else dir.x += random_float() * 0.5f;
		if (dir.z == 0) dir.z = random_float() / 10; else dir.z += random_float() * 0.5f;
		dir.y = random_float() / 10;
		dir = dir * 5.0f;
		particles->add(pos, dir, (rand() % 500) + 250);
		for (int p = 0; p < 3; ++p)
			particles_small->add(pos, dir + glm::vec3(random_float(), 0.5, random_float()), (rand() % 750) + 150);
	}
#endif // A1_8
}

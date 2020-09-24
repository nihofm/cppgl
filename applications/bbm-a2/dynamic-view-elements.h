#pragma once

#include <list>
#include <cppgl.h>

#include "rendering.h"

void init_dynamic_prototypes();

class Player {
	bool moving;
	glm::vec3 move_dir, base;
	float move_duration;

	struct Dir { int x, y; Dir(int x, int y) : x(x), y(y) {} bool operator==(const Dir& d) { return d.x==x && d.y==y; }};
	Dir look_dir;
	float move_rotation;
	float base_rotation;
	float rotation(Dir dir);
	float rotation_angle_between(Dir from, Dir to);

	Timer movement_timer;
    Timer particle_timer;
    Timer wobble_timer;
	
public:
	Player(const std::string &name, int id);

	void update();
	void draw();

	void force_position(int x, int y);
	void start_moving(int dir_x, int dir_y, int est_duration);

    // data
	std::string name;
	glm::mat4 trafo, trafo_rot;
	int health;
	int id;
	bool respawning;
	int frags;
#ifndef A2_3
	static std::vector<glm::vec3> colors;
#endif
    static std::vector<Drawelement> prototype;
};


class Box {
public:
	Box(unsigned posx, unsigned posy, bool isStone);

	void set_exploding(const glm::vec2& dir);
	bool to_destroy();
	
	void draw();

    // data
    glm::mat4   trafo;
	bool        is_stone;
	int         stone_type;
	glm::vec2   uv_offset;
    bool        exploding;
    Timer       explo_timer;
    std::vector<glm::vec3> explo_rot_axis;
    std::vector<glm::vec3> explo_translation;
	static Material wood_material;
	static std::vector<Material> stone_materials;
	static std::vector<Drawelement> prototype_idle;
	static std::vector<Drawelement> prototype_scatter;
};

class Bomb {
public:
	Bomb(int posx, int posy, int id);

	void draw();

    // data
	int x, y, id;
    glm::mat4 trafo;
    static std::vector<Drawelement> prototype;
};


class Board {
	int tiles_x, tiles_y;
	struct Cell {
		int occupied;
        std::shared_ptr<Box> box;
	};
	std::vector<std::vector<Cell>> cells;
	std::vector<std::shared_ptr<Box>> stone_boxes;
	std::list<std::shared_ptr<Box>> crates;
	std::list<std::shared_ptr<Bomb>> bombs;

	// Timer and flag for the camera explosion shake
	Timer camera_shake_timer;
	bool camera_shake;

public:
	Board(int tiles_x, int tiles_y);

	void add_box(int x, int y, int type);
	void add_bomb(int x, int y, int id);

	void explosion(int bomb_id, unsigned int codes);

	void update();
	void draw();
};


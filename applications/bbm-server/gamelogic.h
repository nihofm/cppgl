#ifndef __GAMELOGIC_H__ 
#define __GAMELOGIC_H__ 

#include <string>
#include <deque>

#include <messages/messages.h>

namespace gamelogic {

	namespace settings {
		static const int movement_steps = 30;
		static const int timeslicethickness = 20;
		static const int bomb_steps = 90;
		static const uint8_t bomb_intensity = 60;
		static const int respawn_timeout = 200;
		static const uint8_t initial_player_health = 100;
	}

	class Bomb {
		uint8_t x, y;
		uint16_t id;
		uint8_t dropped_by;
		int remaining_steps;
	public:
		Bomb(uint8_t x, uint8_t y, uint16_t id, uint8_t dropped_by, int steps);
		bool Tick(); //!< returns true on explosion.
	};
	extern std::deque<Bomb*> bombs;


	class Player {
		static uint16_t bomb_counter;
	public:
		struct Pos { 
			uint8_t x, y; 
			Pos() : x(-1), y(-1) {}
			Pos(uint8_t x, uint8_t y) : x(x), y(y) {}
			bool operator==(const Pos &p) { return p.x == x && p.y == y; }
			bool operator!=(const Pos &p) { return !(*this == p); }
		};
		typedef Pos Dir;
	
	private:
		uint8_t id;
		bool alive;
		Pos pos;
		// movement
		int remaining_move_steps;
		bool moving;
		Dir move_dir;
		Dir next_move_dir;
		// bombs
		int bombs_left;
		bool wants_to_drop_bomb;
		int8_t health;	// [0,100], don't use unsigned.
		int respawn_countdown;

		void send_move_message();
		void send_stop_message();
		bool next_move_is_valid();
		void try_to_move();
		void try_to_drop_bomb();
		
		void die(uint8_t by);
	
	public:
		std::string name;
		int8_t frags;

		Player(uint8_t id);
		void Position(uint8_t newx, uint8_t newy);
		Pos Position() const { return pos; }
		bool IsAlive() const { return alive; }
		
		Dir MoveDirection() const     { return move_dir; }
		Dir NextMoveDirection() const { return next_move_dir; }
		void MoveIn(Dir dir);
		void StopMoveingAtNextField();
		void DropBomb();
		void RestoreBomb();

		void Damage(uint8_t dmg, uint8_t by_player);
		
		void Tick(); // make a time step for this player
	};
	extern std::vector<std::unique_ptr<Player>> players;
	extern uint8_t num_players;
	
	struct Box {
		uint8_t x, y;
		msg::box_type::t type;
		Box(uint8_t x, uint8_t y, msg::box_type::t type) : x(x), y(y), type(type) {}
	};


	class Board {
		uint8_t w, h;
		struct field_t {
			int box;
		};
		std::vector<std::vector<field_t>> field;
		int remove_boxes_and_hit_players(uint8_t x, uint8_t y, int dx, int dy, int rest, int maxlen, uint8_t intensity, uint8_t by);
		bool hit_player_at(uint8_t x, uint8_t y, uint8_t damage, uint8_t by);
	public:
		Board(uint8_t w, uint8_t h);
		// ~Board();
		void Print();
		bool FieldOccupied(uint8_t x, uint8_t y) const { return field[y][x].box; }
		void CreateBox(uint8_t x, uint8_t y, int type) { field[y][x].box = type; }
		uint8_t Width() const                      { return w; }
		uint8_t Height() const                     { return h; }
		uint8_t HandleExplosionAt(uint8_t x, uint8_t y, uint8_t by); // returns the possible explosion directions, encoded in an uint.
		Player::Pos FindPosition();
	};
	extern std::unique_ptr<Board> board;



}


#endif


#include "gamelogic.h"

#include <messages/messages.h>

#include <iostream>

using namespace std;

void broadcast(msg::message *m); // server-main.cpp

namespace gamelogic {

	// 
	// the bombs
	// 

	Bomb::Bomb(uint8_t x, uint8_t y, uint16_t id, uint8_t dropped_by, int steps) : x(x), y(y), id(id), dropped_by(dropped_by), remaining_steps(steps) {
		msg::spawn_bomb sb = make_message<msg::spawn_bomb>();
		sb.x = x;
		sb.y = y;
		sb.id = id;
		sb.est_duration = settings::timeslicethickness * settings::bomb_steps;
		broadcast(&sb);
	}

	bool Bomb::Tick() {
		if (remaining_steps-- == 0) {
			msg::bomb_explosion be = make_message<msg::bomb_explosion>();
			uint8_t code = board->HandleExplosionAt(x, y, dropped_by);
			be.dir_codes = code;
			be.id = id;
			broadcast(&be);
			players[dropped_by]->RestoreBomb();
			return true;
		}
		return false;
	}
	
	std::deque<Bomb*> bombs;


	// 
	// the player
	// 

	Player::Player(uint8_t id) 
	: id(id), alive(true), moving(false), move_dir(Dir(0,0)), next_move_dir(Dir(0,0)), bombs_left(3), wants_to_drop_bomb(false),
	  health(settings::initial_player_health), frags(0)
	{
	}
	
	void Player::Position(uint8_t newx, uint8_t newy) { 
		pos.x = newx; 
		pos.y = newy; 
	}
	
	void Player::MoveIn(Dir dir) {
		if (!alive) return;
		next_move_dir = dir;
	}
	
	void Player::StopMoveingAtNextField() {
		next_move_dir.x = next_move_dir.y = 0;
	}

	void Player::send_move_message() {
		msg::start_move sm = make_message<msg::start_move>();
		sm.player = id;
		sm.dir_x = move_dir.x;
		sm.dir_y = move_dir.y;
		sm.est_duration = settings::timeslicethickness * settings::movement_steps;
		broadcast(&sm);
	}

	void Player::send_stop_message() {
		msg::force_player_position fpp = make_message<msg::force_player_position>();
		fpp.player = id;
		fpp.x = pos.x;
		fpp.y = pos.y;
		broadcast(&fpp);
	}

	bool Player::next_move_is_valid() {
		Pos p = pos;
		p.x = uint8_t(p.x + next_move_dir.x);
		p.y = uint8_t(p.y + next_move_dir.y);
		if (p.x >= board->Width() || p.y >= board->Height())
			return false;
		return ! board->FieldOccupied(p.x, p.y);
	}

	void Player::try_to_move() {
		if (next_move_is_valid()) {
			remaining_move_steps = settings::movement_steps;
			move_dir = next_move_dir; // keep next_move_dir for later.
			moving = true;
			send_move_message();
		}
		else {
			moving = false;
			send_stop_message();
			next_move_dir = Dir(0,0);
			send_stop_message();
		}
	}

	void Player::DropBomb() {
		if (!alive) return;
		wants_to_drop_bomb = true;
	}

	void Player::try_to_drop_bomb() {
		if (bombs_left <= 0) return;
		--bombs_left;
		bombs.emplace_back(new Bomb(pos.x, pos.y, bomb_counter++, id, settings::bomb_steps));
	}

	void Player::RestoreBomb() {
		++bombs_left;
	}

	void Player::die(uint8_t by_player) {
		health = 0;
		alive = false;
		next_move_dir.x = next_move_dir.y = 0;
		moving = false;
		send_stop_message();
		respawn_countdown = settings::respawn_timeout;
		if (by_player == id) 
			//frags = std::max(frags-1, 0);
			frags = int8_t(frags-1);
		else {
			//frags = std::max(frags-1, 0);
			//frags = frags-1;
			players[by_player]->frags++;
			msg::frags_update fu = make_message<msg::frags_update>();
			fu.player = by_player;
			fu.frags = players[by_player]->frags;
			broadcast(&fu);
		}
		msg::frags_update fu = make_message<msg::frags_update>();
		fu.player = id;
		fu.frags = frags;
		broadcast(&fu);
	}

	void Player::Damage(uint8_t dmg, uint8_t by_player) {
		health = int8_t(health - dmg);
		if (health <= 0) {
			die(by_player);
		}
		msg::player_health_info phi = make_message<msg::player_health_info>();
		phi.id = id;
		phi.health = health;
		broadcast(&phi);
	}

	void Player::Tick() { // make a time step for this player
		if (!alive) {
			if (--respawn_countdown == 0) {
				pos = board->FindPosition();
				alive = true;
				health = settings::initial_player_health;
				Damage(0,-1); // send resapwn/heath-reset message
				send_stop_message(); // force player on client side to appear at the new position
			}
			return;
		}
		if (moving) {
			--remaining_move_steps;
			bool arrived = false;
			// two part arrival: when we're across the line between two fields we are treated as if we're on the new field.
			// nevertheless, we don't actually "arrive" there.
			if (move_dir != Dir(0,0) && remaining_move_steps == settings::movement_steps/2) { // we arrive
				pos.x = uint8_t(pos.x + move_dir.x);
				pos.y = uint8_t(pos.y + move_dir.y);
			}
			if (move_dir != Dir(0,0) && remaining_move_steps == 0) { // we arrive
				move_dir = Dir(0,0);
				arrived = true;
			}
			if (arrived) {	
				if (next_move_dir != Dir(0,0))
					try_to_move();
				else if (next_move_dir == Dir(0,0)) {
					moving = false;
					send_stop_message();
				}
			}
		}
		else // start a move
			if (next_move_dir != Dir(0,0))
				try_to_move();
		
		if (wants_to_drop_bomb) {
			try_to_drop_bomb();
			wants_to_drop_bomb = false;
		}
	}
	
	uint16_t Player::bomb_counter = 0;
	std::vector<std::unique_ptr<Player>> players;
	uint8_t num_players;


	// 
	// the game board
	// 

	Board::Board(uint8_t w, uint8_t h) : w(w), h(h) {
		field = std::vector<std::vector<field_t>>(h);
		for (uint8_t i = 0; i < h; ++i) {
			field[i] = std::vector<field_t>(w);
			for (uint8_t j = 0; j < w; ++j)
				field[i][j].box = msg::box_type::none;
		}
	}
	
	void Board::Print() {
		cout << "+";
		for (uint8_t x = 0; x < w+2; ++x)
			cout << "-";
		cout << "+" << endl;;

		for (uint8_t y = 0; y < h; ++y) {
			cout << "| ";
			for (uint8_t x = 0; x < w; ++x) {
				bool wrote_player = false;
				for (uint8_t p = 0; p < num_players; ++p)
					if (players[p]->Position() == Player::Pos(x,y)) {
						cout << int(p);
						wrote_player = true;
						break;
					}
				if (wrote_player) continue;

				switch (field[y][x].box) {
					case msg::box_type::none: cout << " "; break;
					case msg::box_type::wood: cout << "°"; break;
					case msg::box_type::stone: cout << "#"; break;
					default: cout << "?"; break;
				}
			}
			cout << " |" << endl;
		}

		cout << "+";
		for (uint8_t x = 0; x < w+2; ++x)
			cout << "-";
		cout << "+" << endl;;
	}
	
	int Board::remove_boxes_and_hit_players(uint8_t x, uint8_t y, int dx, int dy, int rest, int maxlen, uint8_t intensity, uint8_t by_player) {
		if (rest <= 0) return 0;
		hit_player_at(uint8_t(x+dx), uint8_t(y+dy), intensity, by_player);
		if (int(x+dx) < 0 || x+dx >= w || int(y+dy) < 0 || y+dy >= h) return 0;
		if (field[y+dy][x+dx].box == msg::box_type::stone)  return 0;
		if (field[y+dy][x+dx].box == msg::box_type::wood)  { 
			field[y+dy][x+dx].box = msg::box_type::none; 
			return 1; 
		}
		if (field[y+dy][x+dx].box == msg::box_type::none)  
			return 1 + remove_boxes_and_hit_players(uint8_t(x+dx), uint8_t(y+dy), dx, dy, rest-1, maxlen, intensity/2, by_player);
		return -1;
	}
	
	bool Board::hit_player_at(uint8_t x, uint8_t y, uint8_t damage, uint8_t by_player) {
		for (uint8_t i = 0; i < num_players; ++i)
			if (players[i]->Position() == Player::Pos(x, y) && players[i]->IsAlive())
				players[i]->Damage(damage, by_player);
		return false;
	}

	uint8_t Board::HandleExplosionAt(uint8_t x, uint8_t y, uint8_t by_player) {
		int x_pos = remove_boxes_and_hit_players(x, y, 1, 0, 2, 2, settings::bomb_intensity/2, by_player), 
			x_neg = remove_boxes_and_hit_players(x, y,-1, 0, 2, 2, settings::bomb_intensity/2, by_player), 
			y_pos = remove_boxes_and_hit_players(x, y, 0, 1, 2, 2, settings::bomb_intensity/2, by_player), 
			y_neg = remove_boxes_and_hit_players(x, y, 0,-1, 2, 2, settings::bomb_intensity/2, by_player);
		hit_player_at(x,y,settings::bomb_intensity, by_player);
		return uint8_t(x_pos | (x_neg << 2) | (y_pos << 4) | (y_neg << 6));
	}
		
	Player::Pos Board::FindPosition() {
		Player::Pos pp;
		do {
			pp.x =uint8_t (rand() % w);
			pp.y =uint8_t (rand() % h);
		} while (board->FieldOccupied(pp.x, pp.y));
		return pp;
	}

	std::unique_ptr<Board> board = 0;

}


/* vim: set foldmethod=marker: */


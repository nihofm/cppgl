
#include "server-cmdline.h"
#include "gamelogic.h"

#include <cppgl.h>
#include <messages/messages.h>

#include <glm/gtc/type_precision.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;
using namespace gamelogic;

// NOTE: in opengl +y is up, in our array representation +y is down.

void broadcast(msg::message *m);

class server_message_reader : public message_reader {
	uint8_t player_id; // the messages are sent by this player
public:
	server_message_reader(message_reader::socket *sock, uint8_t player) : message_reader(sock), player_id(player) {}
	virtual ~server_message_reader() {};
	void handle_message(msg::key_updown *m) {
		cout << "key down: '" << (int)m->k << "'" << endl;
		Player::Pos p = players[player_id]->Position();
		Player::Dir dir;
		switch (m->k) {
			case msg::key_code::up:    dir = Player::Dir(0,-1); break;
			case msg::key_code::down:  dir = Player::Dir(0,1); break;
			case msg::key_code::left:  dir = Player::Dir(-1,0); break;
			case msg::key_code::right: dir = Player::Dir(1,0); break;
			default: cerr << "invalid move direction code!" << endl; return;
		}
		if (m->down)
			cout << "Moving from " << int(p.x) << "," << int(p.y) << " in direction " << int(dir.x) << "," << int(dir.y) << endl;
		else
			cout << "Stopped moving in direction " << int(dir.x) << "," << int(dir.y) << endl;
		if (m->down)
			players[player_id]->MoveIn(dir);
		else if (players[player_id]->NextMoveDirection() == dir)
			players[player_id]->StopMoveingAtNextField();
	}
	void handle_message(msg::key_drop *m) {
		(void)m;
		cout << "player " << player_id << " tries to drop a bomb" << endl;
		players[player_id]->DropBomb();
	}
	void handle_message(msg::player_name *m) {
		cout << "player " << player_id << " is now known as " << m->name << endl;
		players[player_id]->name = m->name;
		msg::player_name bounce = make_message<msg::player_name>();
		bounce.id = player_id;
		strncpy(bounce.name, m->name, msg::player_name::max_string_length);
		bounce.name[msg::player_name::max_string_length-1] = '\0';
		broadcast(&bounce);
    }
};

namespace client_connections {
	unsigned int sockets = 0;
	boost::asio::ip::tcp::socket **socket;
	server_message_reader **reader;
}

struct quit_signal {
	int status;
	quit_signal(int s) : status(s) {}
};
void quit(int status) { throw quit_signal(status); }

Timer since_last_broadcast;
void broadcast(msg::message *m) {
	unsigned i = 0;
	try {
		for (i = 0; i < client_connections::sockets; ++i)
			boost::asio::write(*client_connections::socket[i], boost::asio::buffer(m, m->message_size), boost::asio::transfer_all());
	}
	catch (boost::system::system_error &err) {
		cerr << "Error sending message to client of player " << players[i]->name << ": " << err.what() << "\nExiting." << endl;
		quit(-1);
	}
	since_last_broadcast.restart();
}


// temporary alternative to loading a png
#include <fstream>
std::vector<Box> boxes;
glm::u8vec2 read_map_file(std::string const& name) {
  std::ifstream ifile(name);

  if(!ifile) {
    std::cerr << "File \'" << name << "\' not found" << std::endl;
    
    throw std::invalid_argument(name);
  }
  
  int row_width = -1;  
  uint8_t num_rows = 0;
  while(ifile.good()) {
    std::string row;
    std::getline(ifile, row);
    // init width
    if (row_width < 0) {
    	row_width = int(row.size());
    }


    else if (row_width != (int)row.size() && !ifile.eof()) {
    	throw std::runtime_error{"map rows have different widths"};
    }

    for(uint8_t x = 0; x < row.size(); ++x) {
			msg::box_type::t type = msg::box_type::none;
			if (row[x] == 's') {
				type = msg::box_type::stone;
			}
			else if (row[x] == 'w') {
				type = msg::box_type::wood;
			}
			if (type != msg::box_type::none) {
    		boxes.emplace_back(Box{x, num_rows, type});
			}
    }
    ++num_rows;
    // filetext.append(row + "\n");
  }
  --num_rows;
  
  return glm::uvec2{row_width, (int)num_rows}; 
}


int main(int argc, char **argv)
{	
	using namespace boost::asio::ip;

	parse_cmdline(argc, argv);
	
	client_connections::sockets = cmdline.players;
	client_connections::socket = new tcp::socket*[client_connections::sockets];
	client_connections::reader = new server_message_reader*[client_connections::sockets];
			
	try
	{
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 2214));

		for (uint8_t i = 0; i < client_connections::sockets; ++i) {
			cout << "waiting for connection " << i << endl;
			client_connections::socket[i] = new tcp::socket(io_service);
			acceptor.accept(*client_connections::socket[i]);
			cout << "got it" << endl;
			client_connections::reader[i] = new server_message_reader(client_connections::socket[i], i);

			msg::connected msg = make_message<msg::connected>();
			msg.your_id = i;
			msg.players = cmdline.players;

			boost::system::error_code ignored_error;
			boost::asio::write(*client_connections::socket[i], boost::asio::buffer(&msg, msg.message_size), boost::asio::transfer_all(), ignored_error);
		}

		glm::u8vec2 res = read_map_file(cmdline.map);		

		board.reset(new Board(res.x, res.y));
		msg::board_info bi = make_message<msg::board_info>();
		bi.w = res.x;
		bi.h = res.y;
		bi.boxes = uint16_t(boxes.size());
		broadcast(&bi);

		for (unsigned i = 0; i < boxes.size(); ++i) {
			msg::spawn_box sb = make_message<msg::spawn_box>();
			sb.x = boxes[i].x;
			sb.y = boxes[i].y;
			sb.box_type = boxes[i].type;
			board->CreateBox(sb.x, sb.y, sb.box_type);
			broadcast(&sb);
		}
		
		num_players = cmdline.players;
		players = std::vector<std::unique_ptr<Player>>(num_players);
		for (uint8_t i = 0; i < num_players; ++i) {
			players[i].reset(new Player(i));
			msg::initial_player_data_position pp = make_message<msg::initial_player_data_position>();
			pp.player = i;
			Player::Pos pos = board->FindPosition();
			pp.x = pos.x;
			pp.y = pos.y;
			broadcast(&pp);
			players[i]->Position(pp.x,pp.y);
			players[i]->Damage(0,-1); // send healt init message.
		}


		msg::start_game sg = make_message<msg::start_game>();
		broadcast(&sg);

		bool game_over = false;

		// main loop
		Timer wtt, info_timer;
		while (1) {
			if (wtt.look() < Timer::msec(settings::timeslicethickness)) {
				sleep(0); // just schedule away...
				continue;
			}
			wtt.start();
			for (unsigned i = 0; i < client_connections::sockets; ++i) {
				client_connections::reader[i]->read_and_handle();
				if (client_connections::reader[i]->eof()) {
					cerr << "Player " << players[i]->name << " disconnected. Exiting." << endl;
					quit(0);
				}
			}

			if (!game_over)
			{
				for (unsigned i = 0; i < num_players; ++i)
					players[i]->Tick();

				int clear = 0;
				for (deque<Bomb*>::iterator it = bombs.begin(); it != bombs.end(); ++it)
					if ((*it)->Tick())
						++clear;
				while (clear-- > 0)      // NOTE: this does no longer work when bombs may have different timeouts!
					bombs.pop_front();

				for (unsigned i = 0; i < num_players; ++i)
					if (players[i]->frags >= cmdline.frag_limit) {
						game_over = true;
						msg::game_over go = make_message<msg::game_over>();
						broadcast(&go);
					}
			}

			if (info_timer.look() > Timer::sec(3)) {
				 board->Print();
				 info_timer.restart();
			}
			if (since_last_broadcast.look() > Timer::sec(1)) {
				msg::keep_alive ping = make_message<msg::keep_alive>();
				broadcast(&ping);
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (quit_signal &sig) {
		exit(sig.status);
	}


	return 0;
}


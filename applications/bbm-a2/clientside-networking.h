#pragma once

#include <messages/messages.h>
#include <string>

void networking_prologue();
void send_message(msg::message& msg);

class client_message_reader : public message_reader {
	bool setup_done;
public:
	client_message_reader(message_reader::socket *sock);

	bool prologue_over();
	virtual ~client_message_reader() {};
protected:
	virtual void handle_message(msg::connected *m);
	virtual void handle_message(msg::initial_player_data_position *m);
	virtual void handle_message(msg::board_info *m);
	virtual void handle_message(msg::spawn_box *m);
	virtual void handle_message(msg::start_game *m);
	virtual void handle_message(msg::force_player_position *m);
	virtual void handle_message(msg::start_move *m);
	virtual void handle_message(msg::spawn_bomb *m);
	virtual void handle_message(msg::bomb_explosion *m);
	virtual void handle_message(msg::player_health_info *m);
	virtual void handle_message(msg::player_name *m);
	virtual void handle_message(msg::frags_update *m);
	virtual void handle_message(msg::game_over *m);
};

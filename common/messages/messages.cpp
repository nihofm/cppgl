#include "messages.h"

#include <iostream>

using namespace std;

message_reader::message_reader(socket *_sock) : got_eof(false), msg_size(0) { 
	sock = _sock;
	 curr_input = buffer; 
}

void message_reader::read_and_handle() {
	// check if data is available
	boost::system::error_code av_error;
	size_t av = sock->available(av_error);
	if (av_error) throw boost::system::system_error(av_error);
	if (av == 0) return;
	// copy data to our buffer
	boost::system::error_code error;
	size_t len = boost::asio::read(*sock, boost::asio::buffer(curr_input, buffer_size-(curr_input-buffer)), boost::asio::transfer_at_least(av), error);
	if (error == boost::asio::error::eof) got_eof = true;
	else if (error)                       throw boost::system::system_error(error);
	curr_input += len;
	// manage received data
	if (msg_size != 0)
		read_message_part();
	while (msg_size == 0 && curr_input - buffer > 0) {
		msg_size = buffer[0];
		read_message_part();
	}
}
	
void message_reader::read_message_part() {
	size_t read = curr_input - buffer;
	if (read >= msg_size && read > 0) {
// 		cout << "read message of size " << msg_size << endl;
		handle_message();
		for (size_t i = msg_size; i < read; ++i)
			buffer[i-msg_size] = buffer[i];
		curr_input = buffer + (read - msg_size);
		msg_size = 0;
	}
}
	
void message_reader::handle_message() {
	switch (buffer[1]) {
		case msg::code::connected:                    handle_message((msg::connected*)buffer); break;
		case msg::code::initial_player_data_position: handle_message((msg::initial_player_data_position*)buffer); break;
		case msg::code::board_info:                   handle_message((msg::board_info*)buffer); break;
		case msg::code::spawn_box:                    handle_message((msg::spawn_box*)buffer); break;
		case msg::code::start_game:                   handle_message((msg::start_game*)buffer); break;
		case msg::code::force_player_position:        handle_message((msg::force_player_position*)buffer); break;
		case msg::code::start_move:                   handle_message((msg::start_move*)buffer); break;
		case msg::code::spawn_bomb:                   handle_message((msg::spawn_bomb*)buffer); break;
		case msg::code::bomb_explosion:               handle_message((msg::bomb_explosion*)buffer); break;
		case msg::code::player_health_info:           handle_message((msg::player_health_info*)buffer); break;
		case msg::code::frags_update:                 handle_message((msg::frags_update*)buffer); break;
		case msg::code::game_over:                    handle_message((msg::game_over*)buffer); break;

		case msg::code::key_updown:                   handle_message((msg::key_updown*)buffer); break;
		case msg::code::key_drop:                     handle_message((msg::key_drop*)buffer); break;
		case msg::code::player_name:                  handle_message((msg::player_name*)buffer); break;

		case msg::code::keep_alive:                   /* we just drop it */ break;
		default:
			cerr << "got a message with unknown code " << (int)buffer[1] << endl;
			exit(-1);
	}
}
void message_reader::warn(msg::message *msg) {
	cout << "got a message of type " << (int)msg->message_type << " for which there is no handler defined!" << endl;
}

/* vim: set foldmethod=marker: */


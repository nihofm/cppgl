#pragma once

#include <string>

//! \brief Translated command line options
struct Cmdline
{
	bool verbose;	//!< wheather or not to be verbose
	std::string server_host, player_name;
	int res_x, res_y;

	Cmdline() : verbose(false), server_host("localhost"), res_x(1280), res_y(720) {
		player_name = "unknown";
	}
};

extern Cmdline cmdline;

int parse_cmdline(int argc, char **argv);


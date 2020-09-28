#include "server-cmdline.h"

#include <string>
#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>
#include <cstdlib>

using namespace std;
using namespace boost::program_options;
namespace po = boost::program_options;

Cmdline cmdline;

int parse_cmdline(int argc, char** argv)
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("verbose,v", "Be verbose.")
		("players,n", po::value<unsigned int>(), "Set number of players.")
		("fraglimit,f", po::value<int>(), "Frags required to win the game.")
		("map,m", po::value<string>(), "Map to be used. Default: default.map")
		;

	//parse
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (vm.count("help")) {
		cout << desc << endl;
		return 0;
	}
	if (vm.count("verbose")) {
		cmdline.verbose = true;
	}
	if (vm.count("players")) {
		unsigned int players = vm["players"].as<unsigned int>();
		if (players > 30) {
			std::cout << "too many players" << std::endl;
			return 0;
		}
		cmdline.players = players;
	}
	if (vm.count("fraglimit")) {
		cmdline.frag_limit = vm["server"].as<int>();
	}
	if (vm.count("map")) {
		std::string map_str = vm["map"].as<string>();
		cmdline.map = map_str;
	}
	std::cout << cmdline.map << std::endl;
	return 1;
}



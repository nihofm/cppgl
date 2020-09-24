#include "cmdline.h"

#include <string>
#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>

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
		("name,n", po::value<string>(), "Set name.")
		("res,r", po::value<string>(), "Resolution. Set as WxH")
		("server,s", po::value<string>(), "Server to connect to.")
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
	if (vm.count("name")) {
		cmdline.player_name = vm["name"].as<string>();
	}
	if (vm.count("server")) {
		cmdline.server_host = vm["server"].as<string>();
	}
	if (vm.count("res")) {
		std::string res_str = vm["res"].as<string>();
		std::string w_str = res_str.substr(0, res_str.find("x", 0));
		std::string h_str = res_str.substr(res_str.find("x") + 1);
		cmdline.res_x = std::stoi(w_str);
		cmdline.res_y = std::stoi(h_str);
	}
	return 1;
}

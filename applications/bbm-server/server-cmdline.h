/* $Id$ */
#ifndef __CMDLINE_H__ 
#define __CMDLINE_H__ 

#include <string>

//! \brief Translated command line options
struct Cmdline
{
	bool verbose;	//!< wheather or not to be verbose
	uint8_t players;
	std::string map;
	int frag_limit;

	Cmdline() : verbose(false), players(1), map("./default.map"), frag_limit(5)
	{
	}
};

extern Cmdline cmdline;

int parse_cmdline(int argc, char **argv);

#endif


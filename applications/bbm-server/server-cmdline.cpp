#include "server-cmdline.h"

#include <argp.h>
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

// const char *argp_program_version = VERSION;

static char doc[]       = "bbm-server" ": description";
static char args_doc[]  = "";

// long option without corresponding short option have to define a symbolic constant >= 300
// enum { FIRST = 300, REMOVE, OPTS };

static struct argp_option options[] = 
{
	// --[opt]		short/const		arg-descr		?		option-descr
	{ "verbose", 'v', 0,         0, "Be verbose." },
	{ "players", 'n', "#players", 0, "Number of players." },
	{ "fraglimit", 'f', "n", 0,      "Frags required to win the game." },
	{ 0 }
};	

string& replace_nl(string &s)
{
	for (unsigned i = 0; i < s.length(); ++i)
		if (s[i] == '\n' || s[i] == '\r')
			s[i] = ' ';
	return s;
}


static error_t parse_options(int key, char *arg, argp_state *state)
{
	// call argp_usage to stop program execution if something is wrong
	(void)state;
	string sarg;
	if (arg)
		sarg = arg;
	sarg = replace_nl(sarg);

	switch (key)
	{
	case 'v':	cmdline.verbose = true; 	break;
	case 'n':	cmdline.players = uint8_t(atoi(arg)); break;
	case 'f':	cmdline.frag_limit = atoi(arg); break;
	
	case ARGP_KEY_ARG:		// process arguments. 
							// state->arg_num gives number of current arg
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp parser = { options, parse_options, args_doc, doc };

int parse_cmdline(int argc, char **argv)
{
	int ret = argp_parse(&parser, argc, argv, /*ARGP_NO_EXIT*/0, 0, 0);
	std::cout << cmdline.map << std::endl;
	return ret;
}
	
Cmdline cmdline;


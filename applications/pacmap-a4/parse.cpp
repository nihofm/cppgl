#include "parse.h"
#include <expat.h>

using namespace std;

/* TODO
 * In our implementation this file contained the xml parser which created the datastructures used for rendering.
 */


/*! \brief a simple manipulator for indentation
 *  use like
 *  <code>
 *  cout << indent(3) << currtag << endl;
 *  </code>
 */
struct indent {
	int depth;
	const std::string tab;
	indent(int depth, const std::string &tab = "\t")
	: depth(depth), tab(tab) {
	}
	friend ostream& operator<<(ostream &out, const indent ind) {
		for (int i = 0; i < ind.depth; ++i)
			out << ind.tab;
		return out;
	}
};

/* A few hints:
 * - map<string,string>
 * - auto (c++11)
 * - atof(3), atoi(3), atoll(3)
 * - string::operator== (don't use string::compare)
 * - ifstream::fail vs ifstream::eof vs ifstream::read
 * - use const string & if possible
 * - exception handling via std::runtime_error
 */

void setup_map(const std::string& filename) {
    (void)filename;
    // TODO parse XML
}

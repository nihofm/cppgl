#include "osm.h"
#include <earcut/earcut.hpp>

using namespace std;

// Point structure used by triangulation library: earcut
struct Point {
	float x, y;
};

namespace mapbox {
	namespace util {
		template <>
			struct nth<0, Point> {
				inline static float get(const Point &t) {
					return t.x;
				};
			};
		template <>
			struct nth<1, Point> {
				inline static float get(const Point &t) {
					return t.y;
				};
			};
	}
}

/* TODO
 * In our implementation this file contained the actual geometry generation and rendering code.
 */

//! this namespace is just a suggestion ;)
namespace osm {
    
    //! this is called externally, once.
    void make_meshes() {
    }

    //! this is called externally
    void draw_meshes(const Shader& shader) {
        (void)shader;
    }
    
    //! this is not properly initialized, of course :)
    bounds current_bounds;
}

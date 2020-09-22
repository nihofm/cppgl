#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>

#include <cppgl.h>

#include "rendering.h"

namespace osm {
    typedef unsigned long long id_t;    //!< HINT: use me.
    typedef float angle_t;              //!< HINT: use me.

    // 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    //

    struct bounds {
        angle_t minlat, maxlat, minlon, maxlon;
    };
    extern bounds current_bounds;

    // 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    //

    // 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    //

    void make_meshes();
    void draw_meshes(const Shader& shader);
}

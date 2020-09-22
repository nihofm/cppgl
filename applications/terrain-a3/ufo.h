#pragma once

#include <cppgl.h>

class Ufo {
public:
    Ufo();
    ~Ufo();

    void draw();

    // data
    glm::mat4 trafo;
	static std::vector<Drawelement> prototype;
};

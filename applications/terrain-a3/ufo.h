#pragma once

#include <cppgl.h>

class Ufo {
public:
    Ufo();
    ~Ufo();

    void draw();

    // data
    glm::mat4 model;
	static std::vector<Drawelement> prototype;
};

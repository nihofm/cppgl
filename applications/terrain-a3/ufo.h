#pragma once

#include <cppgl/meshloader.h>
#include <cppgl/drawelement.h>

class Ufo {
public:
    Ufo();
    ~Ufo();

    void draw();

    // data
    glm::mat4 trafo;
	static std::vector<std::shared_ptr<Drawelement>> prototype;
};

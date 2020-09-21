#pragma once

#include <cppgl/drawelement.h>
#include <cppgl/timer.h>

void init_static_prototypes();
void draw_gui();

class Floor {
public:
	Floor(int w, int h);

	void draw();

    // data
    glm::mat4 trafo;
    static std::shared_ptr<Drawelement> prototype;
};

class Skybox {
public:
	Skybox(int w, int h, int d);

	void draw();

    // data
    glm::mat4 trafo;
    static std::vector<std::shared_ptr<Drawelement>> prototype;
};

class Fog {
public:
    Fog(int w, int h);

    void draw();

    // data
    glm::mat4 trafo;
    Timer fog_timer;
    static std::shared_ptr<Drawelement> prototype;
};

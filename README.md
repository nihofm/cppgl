CppGL Framework
=======

OpenGL framework for use in teaching and research with focus on simplicity and reusability.

## Dependencies

### Ubuntu (22.04)

    apt-get install -y build-essential libwayland-dev libxkbcommon-dev xorg-dev libopengl-dev freeglut3-dev cmake

### Windows

    Visual Studio build tools 2019 or newer and editor with CMake support

## Clone

Use:

    git clone --recursive git@git9.cs.fau.de:renderingframeworks/cppgl.git


Or, after clone use:

    git submodule update --init

## Build

Without examples:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev && cmake --build build --parallel

With examples:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCPPGL_BUILD_EXAMPLES=ON -Wno-dev && cmake --build build --parallel

## Examples

Included is an example rendering application loading a ```.obj``` file from the command line and rendering it with a standard diffuse shader.
If no object file is given, a rgb debug color is displayed.
Also included is a compute shader example, which changes the output color to a greyscale image.
The debug menu can be accessed by pressing ```F1```.

## Issues / Suggestions / Feedback

Please mail to <nikolai.hofmann@fau.de>, <laura.fink@fau.de> or <linus.franke@fau.de>.

CppGL Framework
=======

OpenGL framework for use in teaching and research with focus on simplicity and reusability.

## Dependencies

### Linux

    * Linux C++17 development environment (build-essential, etc)
    * recent GCC with support for std::filesystem
    * OpenGL, GLEW (libglew-dev)
    * GLFW (libglfw3-dev)
    * CMake (cmake)

### Windows

    * Visual Studio 2019 with cmake-build package

## Examples

Included is an example rendering application loading a ```.obj``` file from the command line and rendering it with a standard diffuse shader.
If no object file is given, a rgb debug color is displayed.
Also included is a compute shader example, which changes the output color to a greyscale image.
The debug menu can be accessed by pressing ```F1```.

## Issues / Suggestions / Feedback

Please mail to <nikolai.hofmann@fau.de>, <laura.fink@fau.de> or <linus.franke@fau.de>.

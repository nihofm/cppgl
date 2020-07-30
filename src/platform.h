#pragma once

#ifdef _WIN32
#ifdef BUILD_CPPGL_DLL
#define _API __declspec(dllexport)
#else
#define _API __declspec(dllimport)
#endif
#else
//UNIX, ignore _API

#endif
#pragma once

#ifdef _WIN32
#ifdef BUILD_CPPGL_DLL
#define _API __declspec(dllexport)
#define _API
#else
#define _API __declspec(dllimport)
#endif
#else
//UNIX, ignore _API
#define _API

#endif
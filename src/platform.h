#pragma once

#ifdef _WIN32
	#ifdef BUILD_CPPGL_DLL
		#define _API __declspec(dllexport)
		#define _EXT_API
	#else
		#define _API __declspec(dllimport)
		#define _EXT_API extern
	#endif
#else
	//UNIX, ignore _API
	#define _API
	#define _EXT_API
#endif
#pragma once

// -------------------------------------------
// main cppgl namespace definition

#define CPPGL_NAMESPACE_BEGIN namespace cppgl {
#define CPPGL_NAMESPACE_END }

// -------------------------------------------
// platform-dependent defines

#ifdef _WIN32

// warning 4251: exported (dll) classes need all members to be exported as well; STL does not do that,
#pragma warning(disable: 4251)
// warning 26444: don't declare variables without names; Our named_handle keeps track of these objects
#pragma warning(once: 26444)
// use enum class instead of enum; Assimp causes this warning a lot
#pragma warning(once: 26812)
// multiplication between 4 and 8 byte integer; STB is filled with that
#pragma warning(once: 26451)

#endif

#pragma once
#define UNICODE
#define BOOST_DISABLE_THREADS
#pragma message("Compiling precompiled headers.\n")

#undef min
#undef max

#ifdef INCLUDE_DWM
#include <dwmapi.h>
#endif

#include "math/vec2d.h"

#include <lua/lua.hpp>
#include <luabind/luabind.hpp>

#include <gl/glew.h>
#include <gl/wglew.h>
#include <GL/GL.h>

#include <Box2D/Box2D.h>

#undef min
#undef max

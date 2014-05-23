#pragma once
#define UNICODE
#define BOOST_DISABLE_THREADS
#pragma message("Compiling precompiled headers.\n")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include <vector>

#include <Windows.h>
#include <gdiplus.h>

#undef min
#undef max

#ifdef INCLUDE_DWM
#include <dwmapi.h>
#endif

#include "math/vec2d.h"

#include <boost\pool\object_pool.hpp>

#include <lua/lua.hpp>
#include <luabind/luabind.hpp>

#include <gl/glew.h>
#include <gl/wglew.h>
#include <GL/GL.h>

#include <Box2D/Box2D.h>

#include <freetype\ft2build.h> 
#include FT_FREETYPE_H

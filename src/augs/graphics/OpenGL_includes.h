#pragma once
#include "OpenGL_error.h"

#if BUILD_OPENGL
#include <glad/glad.h>

#if PLATFORM_WINDOWS
#include <GL/GL.h>
#undef min
#undef max
#elif PLATFORM_UNIX
#include <GL/gl.h>
#else
#error "Unsupported platform!"
#endif
#endif

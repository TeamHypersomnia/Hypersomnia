#pragma once
#include "OpenGL_error.h"

#if BUILD_OPENGL
#include <glad/glad.h>

#if PLATFORM_WINDOWS
#include <GL/GL.h>
#undef min
#undef max
#elif PLATFORM_LINUX
#include <GL/gl.h>
#endif
#endif
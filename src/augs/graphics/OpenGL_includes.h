#pragma once
#include "OpenGL_error.h"

#if BUILD_OPENGL
#include <glad/glad.h>

#if USE_GLFW
#include <GLFW/glfw3.h>
#elif PLATFORM_WINDOWS
#include <GL/GL.h>
#undef min
#undef max
#elif PLATFORM_UNIX
#include <GL/gl.h>
#else
#error "Unsupported platform!"
#endif
#endif

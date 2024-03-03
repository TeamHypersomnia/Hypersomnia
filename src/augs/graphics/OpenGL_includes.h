#pragma once
#include "OpenGL_error.h"

#if BUILD_OPENGL

#if !PLATFORM_WEB
#include <glad/glad.h>
#endif

#if USE_SDL2
#include <SDL2/SDL.h>
#if PLATFORM_WEB
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#endif
#elif USE_GLFW
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

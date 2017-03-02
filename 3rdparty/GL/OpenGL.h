#pragma once
#ifdef PLATFORM_WINDOWS
#include <gl/glew.h>
#include <gl/wglew.h>
#include <GL/GL.h>

#undef min
#undef max

#elif PLATFORM_LINUX
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#include <string>

void report_glerr(GLenum __error, std::string location);
#define glerr { report_glerr(glGetError(), __FUNCTION__); };


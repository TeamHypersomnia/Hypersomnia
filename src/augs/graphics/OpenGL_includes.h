#pragma once
#include <glad/glad.h>

#ifdef PLATFORM_WINDOWS
#include <GL/GL.h>
#undef min
#undef max
#elif PLATFORM_LINUX
#include <GL/gl.h>
#endif

#include <string>

void report_glerr(
	const GLenum __error, 
	const std::string& location
);

#define glerr { report_glerr(glGetError(), __FUNCTION__); };
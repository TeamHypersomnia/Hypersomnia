#pragma once
#include <gl/glew.h>
#include <gl/wglew.h>
#include <GL/GL.h>
#include <string>

void report_glerr(GLenum __error, std::string location);
#define glerr { report_glerr(glGetError(), __FUNCTION__); }
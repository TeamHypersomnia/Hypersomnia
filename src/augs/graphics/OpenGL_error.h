#pragma once
#if BUILD_OPENGL
void check_OpenGL_error(const char* stmt, const char* fname, int line);
#define GL_CHECK(stmt) ((void)(stmt), check_OpenGL_error(#stmt, __FILE__, __LINE__));
#else
#define GL_CHECK(stmt)
#endif

#pragma once
#if BUILD_OPENGL
	#if IS_PRODUCTION_BUILD || PLATFORM_WEB
		#define GL_CHECK(stmt) (void)(stmt);
	#else
		void check_OpenGL_error(const char* stmt, const char* fname, int line);
		#define GL_CHECK(stmt) ((void)(stmt), check_OpenGL_error(#stmt, __FILE__, __LINE__));
	#endif
#else
	#define GL_CHECK(stmt)
#endif

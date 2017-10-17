#include "augs/graphics/OpenGL_includes.h"
#include "augs/log.h"
#include "augs/ensure.h"

static const char * get_OpenGL_error_string(int errID) {
	if (errID == GL_NO_ERROR) return "";
	if (errID == GL_INVALID_ENUM) return " Invalid enum ";
	if (errID == GL_INVALID_VALUE) return " Invalid value ";
	if (errID == GL_INVALID_OPERATION) return " Invalid operation ";
	if (errID == GL_OUT_OF_MEMORY) return " Out of memory like! ";

	return " Don't know ";
}

void check_OpenGL_error(const char* stmt, const char* fname, int line) {
	const GLenum err { glGetError() };

	if (err != GL_NO_ERROR) {
		LOG("OpenAL error %x, (%x) at %x:%x - for %x", err, get_OpenGL_error_string(err), fname, line, stmt);
#if !IS_PRODUCTION_BUILD
		ensure(false && "OpenGL error.");
#endif
	}
}

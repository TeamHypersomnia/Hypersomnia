#include "OpenGL.h"
#include "log.h"

void report_glerr(GLenum __error, std::string location) {
	if (__error) {
		LOG("OpenGL error %x in %x", __error, location);
	}
}

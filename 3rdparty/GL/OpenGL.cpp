#include "3rdparty/GL/OpenGL.h"
#include "augs/log.h"

void report_glerr(
	const GLenum __error, 
	const std::string& location
) {
	if (__error) {
		LOG("OpenGL error %x in %x", __error, location);
	}
}

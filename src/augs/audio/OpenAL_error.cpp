#include <AL/al.h>

#include "augs/audio/OpenAL_error.h"
#include "augs/log.h"
#include "augs/ensure.h"

static const char * get_OpenAL_error_string(int errID) {
	if (errID == AL_NO_ERROR) return "";
	if (errID == AL_INVALID_NAME) return "Invalid name";
	if (errID == AL_INVALID_ENUM) return " Invalid enum ";
	if (errID == AL_INVALID_VALUE) return " Invalid value ";
	if (errID == AL_INVALID_OPERATION) return " Invalid operation ";
	if (errID == AL_OUT_OF_MEMORY) return " Out of memory like! ";

	return " Don't know ";
}

void check_OpenAL_error(const char* stmt, const char* fname, int line) {
	const ALenum err { alGetError() };

	if (err != AL_NO_ERROR) {
		LOG("OpenAL error %x, (%x) at %x:%x - for %x", err, get_OpenAL_error_string(err), fname, line, stmt);
#if !IS_PRODUCTION_BUILD
		ensure(false && "OpenAL error.");
#endif
	}
};
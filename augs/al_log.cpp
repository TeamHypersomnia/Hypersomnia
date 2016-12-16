#include "al_log.h"
#include "ensure.h"
#include <AL/al.h>

const char * GetOpenALErrorString(int errID) {
	if (errID == AL_NO_ERROR) return "";
	if (errID == AL_INVALID_NAME) return "Invalid name";
	if (errID == AL_INVALID_ENUM) return " Invalid enum ";
	if (errID == AL_INVALID_VALUE) return " Invalid value ";
	if (errID == AL_INVALID_OPERATION) return " Invalid operation ";
	if (errID == AL_OUT_OF_MEMORY) return " Out of memory like! ";

	return " Don't know ";
}

void CheckOpenALError(const char* stmt, const char* fname, int line) {
	ALenum err = alGetError();

	if (err != AL_NO_ERROR) {
		LOG("OpenAL error %x, (%x) at %x:%x - for %x", err, GetOpenALErrorString(err), fname, line, stmt);
		ensure(false);
	}
};
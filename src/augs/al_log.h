#pragma once
#include "log.h"
#include "generated/setting_build_openal.h"

#if BUILD_OPENAL
const char * GetOpenALErrorString(int errID);
void CheckOpenALError(const char* stmt, const char* fname, int line);

#ifndef AL_CHECK
#define AL_CHECK(stmt) (CheckOpenALError(#stmt, __FILE__, __LINE__), (stmt))
#endif
#else
#define AL_CHECK(...)
#endif
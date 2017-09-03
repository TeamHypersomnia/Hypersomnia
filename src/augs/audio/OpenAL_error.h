#pragma once
#if BUILD_OPENAL
void check_OpenAL_error(const char* stmt, const char* fname, int line);
#define AL_CHECK(stmt) ((stmt), check_OpenAL_error(#stmt, __FILE__, __LINE__))
// TODO: Implement error checking
#define AL_CHECK_DEVICE(device)
#else
#define AL_CHECK(stmt)
#endif
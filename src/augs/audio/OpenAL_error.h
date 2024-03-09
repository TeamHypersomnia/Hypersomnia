#pragma once
#if BUILD_OPENAL
	#if IS_PRODUCTION_BUILD || PLATFORM_WEB
		#define AL_CHECK(stmt) (void)(stmt);
	#else
		void check_OpenAL_error(const char* stmt, const char* fname, int line);
		#define AL_CHECK(stmt) ((void)(stmt), check_OpenAL_error(#stmt, __FILE__, __LINE__))
	#endif
#else
	#define AL_CHECK(stmt)
#endif

// TODO: Implement error checking

#define AL_CHECK_DEVICE(device)
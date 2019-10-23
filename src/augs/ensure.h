#pragma once
#include "augs/build_settings/setting_enable_ensure.h"
#include "augs/build_settings/compiler_defines.h"

void log_ensure(const char* expr, const char* file, int line);

#if ENABLE_ENSURE && !FORCE_DISABLE_ENSURE
#define ensure(x) if(Unlikely(!(x)))\
{\
	log_ensure(#x, __FILE__, __LINE__ ); \
}
#else
#define ensure(x)
#endif

#pragma once
#include "augs/log.h"
#include "augs/build_settings.h"

void cleanup_proc();

#if ENABLE_ENSURE
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#else
#define ensure(x) (x)
#endif

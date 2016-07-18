#pragma once
#include "log.h"

void cleanup_proc();

#define ENABLE_ENSURE 1

#if ENABLE_ENSURE
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#else
#define ensure(x) (x)
#endif

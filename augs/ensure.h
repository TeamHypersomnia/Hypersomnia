#pragma once
#include <cassert>
#include "log.h"

void cleanup_proc();

#if _DEBUG || NDEBUG
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%s) failed\nFile: %s\nLine: %d", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#else
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%s) failed\nFile: %s\nLine: %d", #x, __FILE__, __LINE__ );\
}
#endif
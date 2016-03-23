#pragma once
#include "log.h"

void cleanup_proc();

#if _DEBUG || NDEBUG
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#else
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nFile: %x\nLine: %x", #x, __FILE__, __LINE__ );\
}
#endif
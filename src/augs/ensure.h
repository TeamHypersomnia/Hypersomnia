#pragma once
#include "augs/log.h"
#include "augs/build_settings/setting_enable_ensure.h"

void cleanup_proc();

#if ENABLE_ENSURE && !FORCE_DISABLE_ENSURE
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#define ensure_eq(expected, actual) if(!(expected == actual))\
{\
    LOG( "ensure_eq(%x, %x) failed:\nexpected: %x\nactual: %x\nfile: %x\nline: %x", #expected, #actual, expected, actual, __FILE__, __LINE__ );\
	cleanup_proc(); \
}
#define should(x) if(!(x))\
{\
    LOG( "should(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
}
#define should_eq(expected, actual) if(!(expected == actual))\
{\
    LOG( "should_eq(%x, %x) failed:\nexpected: %x\nactual: %x\nfile: %x\nline: %x", #expected, #actual, expected, actual, __FILE__, __LINE__ );\
}
#else
#define ensure(x) (x)
#define ensure_eq(x, y) (x, y)
#define should(x) (x)
#define should_eq(x, y) (x, y)
#endif

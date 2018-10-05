#pragma once
#include "augs/log.h"
#include "augs/build_settings/setting_enable_ensure.h"

extern void (*ensure_handler)();

void save_log_and_terminate();

#if ENABLE_ENSURE && !FORCE_DISABLE_ENSURE
#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
}
#define ensure_eq(expected, actual) if(!(expected == actual))\
{\
    LOG( "ensure_eq(%x, %x) failed:\nexpected: %x\nactual: %x\nfile: %x\nline: %x", #expected, #actual, expected, actual, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
}
#define ensure_less(actual, from) if(!(actual < from))\
{\
    LOG( "ensure_less(%x, %x) failed with expansion:\n%x < %x\nfile: %x\nline: %x", #actual, #from, actual, from, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
}
#define ensure_leq(actual, from) if(!(actual <= from))\
{\
    LOG( "ensure_leq(%x, %x) failed with expansion:\n%x <= %x\nfile: %x\nline: %x", #actual, #from, actual, from, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
}
#define ensure_greater(actual, from) if(!(actual > from))\
{\
	LOG( "ensure_greater(%x, %x) failed with expansion:\n%x > %x\nfile: %x\nline: %x", #actual, #from, actual, from, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
}
#define ensure_geq(actual, from) if(!(actual >= from))\
{\
	LOG( "ensure_geq(%x, %x) failed with expansion:\n%x >= %x\nfile: %x\nline: %x", #actual, #from, actual, from, __FILE__, __LINE__ );\
	save_log_and_terminate(); \
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
#define ensure(x)
#define ensure_eq(x, y)
#define should(x)
#define should_eq(x, y)
#endif

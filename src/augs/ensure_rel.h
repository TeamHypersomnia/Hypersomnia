#pragma once
#include "augs/build_settings/setting_enable_ensure.h"
#include "augs/build_settings/compiler_defines.h"

template <class T>
void log_ensure_rel(int type, const char* left_name, const char* right_name, const T& left, const T& right, const char* file, int line);

template <class T>
void log_ensure_rel(int type, const char* left_name, const char* right_name, T* left, T* right, const char* file, int line) {
	log_ensure_rel(type, left_name, right_name, static_cast<const void*>(left), static_cast<const void*>(right), file, line);
}

#if ENABLE_ENSURE && !FORCE_DISABLE_ENSURE
#define ensure_eq(expected, actual) if(Unlikely(!(expected == actual)))\
{\
	log_ensure_rel(0, #expected, #actual, expected, actual, __FILE__, __LINE__); \
}
#define ensure_less(actual, from) if(Unlikely(!(actual < from)))\
{\
	log_ensure_rel(1, #actual, #from, actual, from, __FILE__, __LINE__ ); \
}
#define ensure_leq(actual, from) if(Unlikely(!(actual <= from)))\
{\
	log_ensure_rel(2, #actual, #from, actual, from, __FILE__, __LINE__ ); \
}
#define ensure_greater(actual, from) if(Unlikely(!(actual > from)))\
{\
	log_ensure_rel(3, #actual, #from, actual, from, __FILE__, __LINE__ ); \
}
#define ensure_geq(actual, from) if(Unlikely(!(actual >= from)))\
{\
	log_ensure_rel(4, #actual, #from, actual, from, __FILE__, __LINE__ ); \
}
#else
#define ensure_eq(x, y)
#define ensure_less(x, y)
#define ensure_leq(x, y)
#define ensure_greater(x, y)
#define ensure_geq(x, y)
#endif

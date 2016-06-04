#pragma once
#include "augs/window_framework/colored_print.h"
#include "augs/misc/typesafe_sprintf.h"
#include "build_settings.h"

template < typename... A >
void LOG(std::string f, A&&... a) {
	LOG(typesafe_sprintf(f, std::forward<A>(a)...));
}

template < typename... A >
void LOG_COLOR(console_color color, std::string f, A&&... a) {
	LOG_COLOR(color, typesafe_sprintf(f, std::forward<A>(a)...));
}

template <>
void LOG(std::string f);

template <>
void LOG_COLOR(console_color color, std::string f);

void CALL_SHELL(std::string);

#define AS_INTV *(vec2i*)&
#define AS_INT *(int*)&

#if ENABLE_DEBUG_LOG
#define DEBUG_LOG LOG
#else
#define DEBUG_LOG(...)
#endif
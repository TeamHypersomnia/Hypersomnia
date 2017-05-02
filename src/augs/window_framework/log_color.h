#pragma once
#include "augs/log.h"

template <typename... A>
void LOG_COLOR(const console_color color, const std::string& f, A&&... a) {
	LOG_COLOR(color, typesafe_sprintf(f, std::forward<A>(a)...));
}

template <>
void LOG_COLOR(const console_color color, const std::string& f);

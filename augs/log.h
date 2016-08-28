#pragma once
#include "augs/window_framework/colored_print.h"
#include "augs/misc/typesafe_sprintf.h"
#include "build_settings.h"

#include "augs/gui/formatted_text.h"

#include <queue>

struct log_entry {
	console_color color;
	std::string text;
};

struct global_log {
	static std::vector<log_entry> recent_entries;
	static unsigned max_entries;

	static augs::gui::text::fstr format_recent_as_text(assets::font_id);

	static void push_entry(log_entry);
};

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
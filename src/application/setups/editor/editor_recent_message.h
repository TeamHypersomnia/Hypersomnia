#pragma once
#include "augs/misc/time_utils.h"

struct editor_recent_message {
	std::string content;
	augs::date_time stamp;

	template <class T>
	void set(T&& t) {
		content = std::forward<T>(t);
		stamp = augs::date_time();
	}
};

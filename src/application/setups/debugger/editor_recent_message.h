#pragma once
#include "augs/misc/time_utils.h"

struct editor_recent_message {
	std::string content;
	augs::date_time stamp;

	template <class... Args>
	void set(Args&&... args) {
		content = typesafe_sprintf("%x", std::forward<Args>(args)...);
		stamp = augs::date_time();
	}
};

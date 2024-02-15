#pragma once
#include "augs/misc/date_time.h"

struct debugger_recent_message {
	std::string content;
	augs::date_time stamp;

	template <class... Args>
	void set(Args&&... args) {
		content = typesafe_sprintf("%x", std::forward<Args>(args)...);
		stamp = augs::date_time();
	}
};

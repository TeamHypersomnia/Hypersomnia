#pragma once
#include "augs/misc/date_time.h"
#include "augs/misc/timing/timer.h"

struct editor_recent_message {
	std::string content;
	augs::date_time stamp;
	unsigned show_for_at_least_ms = 0;

	template <class... Args>
	void set(Args&&... args) {
		content = typesafe_sprintf("%x", std::forward<Args>(args)...);
		stamp = augs::date_time();
		show_for_at_least_ms = 0;
	}
};

/*
	A prominent, self-dismissing notice at the top of the screen,
	for events the user should notice but not have to acknowledge.
*/

struct editor_toast {
	std::string content;
	augs::timer since;
	double show_for_secs = 0.0;
	bool shown_yet = false;

	void set(const std::string& new_content, const double secs) {
		content = new_content;
		show_for_secs = secs;
		shown_yet = false;
	}

	/*
		Counts from the first drawn frame,
		so that a long loading doesn't eat the toast.
	*/

	void mark_shown() {
		if (!shown_yet) {
			shown_yet = true;
			since.reset();
		}
	}

	double get_secs_left() const {
		return show_for_secs - since.get<std::chrono::seconds>();
	}
};

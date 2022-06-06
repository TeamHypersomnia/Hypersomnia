#pragma once
#include "augs/misc/timing/timer.h"
#include "application/setups/debugger/editor_settings.h"

struct editor_significant;

namespace sol {
	class state;
}

void open_last_folders(
	sol::state& lua,
	editor_significant& signi
);

class editor_autosave {
	augs::timer autosave_timer;
	editor_autosave_settings last_settings;

public:
	void advance(
		sol::state& lua,
		const editor_significant& signi,
		const editor_autosave_settings&
	);

	void save(
		sol::state& lua,
		const editor_significant& signi
	) const;
};


#pragma once
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/misc/timing/timer.h"
#include "application/setups/editor/editor_settings.h"

struct editor_significant;

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


#pragma once
#include "augs/misc/timing/timer.h"
#include "application/setups/debugger/debugger_settings.h"

struct debugger_significant;

namespace sol {
	class state;
}

void open_last_folders(
	sol::state& lua,
	debugger_significant& signi
);

class debugger_autosave {
	augs::timer autosave_timer;
	debugger_autosave_settings last_settings;

public:
	void advance(
		sol::state& lua,
		const debugger_significant& signi,
		const debugger_autosave_settings&
	);

	void save(
		sol::state& lua,
		const debugger_significant& signi
	) const;
};


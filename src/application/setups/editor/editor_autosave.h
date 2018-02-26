#pragma once
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/misc/timing/timer.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_popup.h"

struct editor_significant;

std::optional<editor_popup> open_intercosm(intercosm& work, const intercosm_path_op op);

template <class derived>
class current_tab_access_cache;
class editor_setup;

std::optional<editor_popup> open_last_tabs(
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


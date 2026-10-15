#pragma once
#include "application/gui/browser_tab.h"

/*
	What the menus remember between launches - gui_state.json in the user folder,
	saved together with the runtime prefs (and on exit), not on every click.
*/

struct gui_state {
	// GEN INTROSPECTOR struct gui_state
	browser_tab server_browser_tab = browser_tab::FAVORITES;
	// END GEN INTROSPECTOR

	bool operator==(const gui_state&) const = default;
};

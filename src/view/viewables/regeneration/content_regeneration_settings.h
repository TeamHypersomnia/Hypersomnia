#pragma once
#include "augs/templates/maybe.h"

struct content_regeneration_settings {
	// GEN INTROSPECTOR struct content_regeneration_settings
	bool regenerate_every_time = false;
	bool rescan_assets_on_window_focus = true;

	augs::maybe<unsigned> custom_resource_workers = augs::maybe<unsigned>(2, false);
	// END GEN INTROSPECTOR

	bool operator==(const content_regeneration_settings& b) const = default;

	/*
		How many workers augs::resource_workers spawns besides the thread that asked
		for the work, which takes part as well. Atlas blitting, neon map regeneration
		and sound decoding all draw from that one pool.
	*/
	unsigned get_resource_workers() const;
	static unsigned get_default_resource_workers();
};

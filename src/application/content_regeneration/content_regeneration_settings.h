#pragma once
#include "application/content_regeneration/buttons_with_corners.h"

struct content_regeneration_settings {
	// GEN INTROSPECTOR struct content_regeneration_settings
	bool check_integrity_every_launch = true;
	bool save_regenerated_atlases_as_binary = true;
	bool regenerate_every_launch = false;
	unsigned packer_detail_max_atlas_size = 8192;

	button_with_corners_input menu_button;
	button_with_corners_input hotbar_button;
	// END GEN INTROSPECTOR
};
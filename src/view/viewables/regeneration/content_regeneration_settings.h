#pragma once

struct content_regeneration_settings {
	// GEN INTROSPECTOR struct content_regeneration_settings
	bool check_integrity_every_launch = true;
	bool save_regenerated_atlases_as_binary = true;
	bool regenerate_every_launch = false;
	unsigned packer_detail_max_atlas_size = 8192;
	// END GEN INTROSPECTOR
};
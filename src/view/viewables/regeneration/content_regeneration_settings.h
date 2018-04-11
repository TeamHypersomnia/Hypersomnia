#pragma once

struct content_regeneration_settings {
	// GEN INTROSPECTOR struct content_regeneration_settings
	bool regenerate_every_launch = false;
	bool skip_source_image_integrity_check = false;

	bool save_regenerated_atlases_as_binary = true;
	unsigned packer_detail_max_atlas_size = 8192;
	// END GEN INTROSPECTOR
};
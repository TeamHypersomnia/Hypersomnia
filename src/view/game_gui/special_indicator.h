#pragma once

struct special_indicator {
	transformr transform;
	rgba color;

	augs::atlas_entry offscreen_tex;
	augs::atlas_entry radar_tex;
};

struct special_indicator_meta {
	entity_id bomb_owner;
	entity_id now_defusing;
};

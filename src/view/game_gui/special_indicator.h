#pragma once

struct special_indicator {
	transformr transform;
	rgba color;

	augs::atlas_entry offscreen_tex;
	augs::atlas_entry radar_tex;

	bool draw_onscreen = true;

	augs::maybe<rgba> draw_nicknames_for_fallback_color = augs::maybe<rgba>(rgba::zero, false);
};

struct special_indicator_meta {
	entity_id bomb_owner;
	entity_id now_defusing;
};

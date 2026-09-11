#pragma once

/*
	camera_query_aabb_add expands the queried camera AABB
	by this many world units on each side.
	Neon maps reach beyond their sprites' AABBs
	(the generation radius is usually 80, e.g. the armor glows on characters),
	so without this expansion their bloom would pop in
	only once the sprite itself entered the camera.
*/

struct session_settings {
	// GEN INTROSPECTOR struct session_settings
	bool show_performance = false;
	bool show_logs = false;
	bool hide_settings_ingame = false;
	float camera_query_aabb_mult = 0.1f;
	float camera_query_aabb_add = 100.0f;
	// END GEN INTROSPECTOR

	bool operator==(const session_settings& b) const = default;
};
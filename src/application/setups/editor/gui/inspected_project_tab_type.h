#pragma once

enum class inspected_project_tab_type {
	ARENA,
	PLAYTESTING,
	ABOUT
};

enum class project_arena_pane {
	// GEN INTROSPECTOR enum class project_arena_pane
	GENERAL,
	LIGHTING,
	BOTS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class project_lighting_pane {
	// GEN INTROSPECTOR enum class project_lighting_pane
	LIGHTS,
	SUN_SHADOWS,
	POINT_LIGHT_SHADOWS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class project_bots_pane {
	// GEN INTROSPECTOR enum class project_bots_pane
	BOT_WAYPOINTS,
	NAVMESH,

	COUNT
	// END GEN INTROSPECTOR
};

enum class project_about_pane {
	// GEN INTROSPECTOR enum class project_about_pane
	DESCRIPTION,
	SCREENSHOTS,
	CLEANUP,

	COUNT
	// END GEN INTROSPECTOR
};

/*
	Which part of the arena settings the Arena tab shows - by its pane and the Lighting or Bots pane's own.
*/

enum class arena_settings_section {
	GENERAL,
	LIGHTS,
	SUN_SHADOWS,
	POINT_LIGHT_SHADOWS,
	BOT_WAYPOINTS,
	NAVMESH
};

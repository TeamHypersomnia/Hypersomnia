#pragma once

struct session_settings {
	// GEN INTROSPECTOR struct session_settings
	bool show_performance = false;
	bool show_logs = false;
	bool hide_settings_ingame = false;
#if TODO
	bool use_system_cursor_for_gui = false;
#endif
	float camera_query_aabb_mult = 0.1f;
	// END GEN INTROSPECTOR
};
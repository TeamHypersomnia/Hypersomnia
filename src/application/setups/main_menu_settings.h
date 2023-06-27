#pragma once
#include "augs/filesystem/path.h"

struct main_menu_settings {
	// GEN INTROSPECTOR struct main_menu_settings
	augs::path_type menu_intro_scene_entropy_path;
	augs::path_type menu_background_arena_path;
	
	augs::path_type menu_theme_path;

	double rewind_intro_scene_by_secs = 3.5;
	double start_menu_music_at_secs = 0.f;

	bool skip_credits = false;
	std::string latest_news_url;
	// END GEN INTROSPECTOR
};
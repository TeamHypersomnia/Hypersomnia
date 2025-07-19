#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/graphics/texture.h"
#include "view/necessary_resources.h"

struct leaderboards_gui_internal;

struct leaderboards_input {
	const std::string nickname;
	const std::string our_id;

	const std::string provider_url;

	const necessary_images_in_atlas_map& necessary_images;
	augs::graphics::texture& avatar_preview_tex;

	const ltrb menu_ltrb;
#if PLATFORM_WEB
	const bool is_logged_in;
	const bool is_crazygames;
#endif
};

struct leaderboards_entry {
	// GEN INTROSPECTOR struct leaderboards_entry
	std::string account_id;
	std::string nickname;
	float mmr;
	// END GEN INTROSPECTOR

	bool is_us = false;
};

struct all_leaderboards {
	// GEN INTROSPECTOR struct all_leaderboards
	std::vector<leaderboards_entry> leaderboards_team;
	std::vector<leaderboards_entry> leaderboards_ffa;
	// END GEN INTROSPECTOR

	void clear() {
		leaderboards_team.clear();
		leaderboards_ffa.clear();
	}
};

enum class leaderboards_type {
	TEAM,
	FFA
};

class leaderboards_gui_state : public standard_window_mixin<leaderboards_gui_state> {
	std::unique_ptr<leaderboards_gui_internal> data;

	std::string last_checked_id;
	std::optional<std::string> requested_connection;
	std::string displayed_connecting_server_name;

	std::string error_message;

	// bool scroll_once_to_selected = false;
	bool refresh_requested = true;
	bool refresh_in_progress() const;

	void refresh_leaderboards(leaderboards_input);

	leaderboards_type type = leaderboards_type::TEAM;
	all_leaderboards all;
	bool refreshed_once = false;

public:
	bool wants_sign_in = false;
	bool wants_log_out = false;

	using base = standard_window_mixin<leaderboards_gui_state>;

	leaderboards_gui_state(const std::string& title);
	~leaderboards_gui_state();

	void perform(leaderboards_input);

	void request_refresh() { refresh_requested = true; }
};

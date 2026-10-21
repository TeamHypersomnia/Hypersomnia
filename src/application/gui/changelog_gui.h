#pragma once
#include <string>
#include <unordered_set>
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "application/gui/changelog_entry.h"

/*
	Switching away from a version clears its (NEW!) for this session, except for the newest release, which stays (NEW!).
	The config remembers only the newest version acknowledged with OK.
*/

struct changelog_view_state {
	std::size_t selected = 0;
	std::unordered_set<std::string> read_versions;

	/*
		"<version>#<line index>" of the expanded top-level points.
	*/

	std::unordered_set<std::string> expanded_items;
};

struct changelog_input {
	const changelog_entries& entries;
	changelog_view_state& state;

	/*
		As it was when the game started - see work.cpp.
	*/

	const std::string& last_seen_version;

	/*
		"Steam reviews help us grow." beside OK, opening the Steam review page - for a Steam account
		(the Steam build, or a key linked to Steam) until a changelog in which it was clicked is closed.
	*/

	const bool show_steam_review;
};

/*
	Rainbow for "(NEW!)" labels across the GUI.
*/

rgba get_new_feature_color();

std::string get_newest_changelog_version(const changelog_entries&);
bool should_show_changelog(const changelog_entries&, const std::string& last_seen_version);
std::size_t oldest_unseen_changelog_index(const changelog_entries&, const std::string& last_seen_version);

struct changelog_window_state : public standard_window_mixin<changelog_window_state> {
	using base = standard_window_mixin<changelog_window_state>;
	using base::base;

	/*
		For the thumb on the Steam review button. Set by the game.
	*/

	const necessary_images_in_atlas_map* necessary_images = nullptr;

	/*
		Set when the review page was opened - the game saves it to the config once the changelog closes.
	*/

	bool steam_review_opened = false;

	bool perform(changelog_input);
};

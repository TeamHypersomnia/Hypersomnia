#pragma once
#include <string>
#include <unordered_set>
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/gui/changelog_entry.h"

/*
	Shared by the changelog shown in place of the leaderboards
	and by the modal opened from Settings -> General.

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
};

/*
	Rainbow for "(NEW!)" labels across the GUI.
*/

rgba get_new_feature_color();

std::string get_newest_changelog_version(const changelog_entries&);
bool should_show_changelog(const changelog_entries&, const std::string& last_seen_version);

/*
	Fills the remaining space of the current window.
	Returns true when OK was pressed.
*/

bool perform_changelog_view(changelog_input);

struct changelog_window_state : public standard_window_mixin<changelog_window_state> {
	using base = standard_window_mixin<changelog_window_state>;
	using base::base;

	bool perform(changelog_input);
};

#pragma once
#include <string>
#include <vector>

#include "augs/filesystem/path.h"
#include "augs/graphics/vertex.h"

#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_commanded_state.h"

using folder_index = unsigned;
constexpr unsigned dead_folder_v = static_cast<folder_index>(-1);

struct editor_popup;
using editor_warning = editor_popup;

namespace sol {
	class state;
}

struct editor_recent_paths;
struct editor_paths;

struct editor_folder {
	editor_folder(const augs::path_type& p = {});

	augs::path_type current_path;

	std::unique_ptr<editor_commanded_state> commanded;

	editor_view view;
	editor_player player;
	editor_history history;

	/* Opened game mode definitions go here */

	void set_folder_path(const augs::path_type&);
	std::string get_display_path() const;

	bool is_untitled() const;

	void save_folder() const;
	void export_folder(sol::state& lua, const augs::path_type& to) const;

	std::optional<editor_warning> open_most_relevant_content(sol::state& lua);

	void mark_as_just_saved();
	bool empty() const;

	bool allow_close() const;

	editor_paths get_paths() const;

	entity_id get_viewed_character_id() const;

	void autosave_if_needed() const;

private:
	void import_folder(sol::state& lua, const augs::path_type& from);

	void load_folder();
	void load_folder(const augs::path_type& from);
	void load_folder(const augs::path_type& from, const augs::path_type& name);

	void save_folder(const augs::path_type& to) const;
	void save_folder(const augs::path_type& to, const augs::path_type name) const;

	bool should_autosave() const;
	augs::path_type get_autosave_path() const;
};

struct editor_last_folders {
	// GEN INTROSPECTOR struct editor_last_folders
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR
};
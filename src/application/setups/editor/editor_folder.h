#pragma once
#include <string>
#include <vector>

#include "augs/filesystem/path.h"
#include "augs/graphics/vertex.h"

#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_commanded_state.h"

using folder_index = unsigned;

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

	void set_folder_path(sol::state&, const augs::path_type&, editor_recent_paths&);
	std::string get_display_path() const;
	augs::path_type get_autosave_path() const;

	bool is_untitled() const;

	bool at_unsaved_revision() const;

	void save_folder() const;
	void save_folder(const augs::path_type& to) const;
	void save_folder(const augs::path_type& to, const augs::path_type name) const;

	std::optional<editor_warning> load_folder_maybe_autosave();

	void load_folder();
	void load_folder(const augs::path_type& from);
	void load_folder(const augs::path_type& from, const augs::path_type& name);

	editor_paths get_paths() const;

	template <class C>
	auto make_player_advance_input(const C& with_callbacks);
};

struct editor_last_folders {
	// GEN INTROSPECTOR struct editor_last_folders
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR
};
#pragma once
#include <string>
#include <unordered_set>
#include <vector>
#include <optional>

#include "augs/filesystem/path.h"
#include "augs/graphics/vertex.h"
#include "augs/math/camera_cone.h"

#include "game/transcendental/entity_id.h"

#include "application/setups/editor/editor_history.h"

using folder_index = unsigned;

namespace sol {
	class state;
}

struct intercosm;
struct editor_recent_paths;
struct editor_paths;

struct editor_folder_meta {
	// GEN INTROSPECTOR struct editor_folder_meta
	augs::timer timestamp;
	// END GEN INTROSPECTOR
};

struct editor_view {
	// GEN INTROSPECTOR struct editor_view
	editor_folder_meta meta;

	std::unordered_set<entity_id> selected_entities;
	std::optional<camera_cone> panned_camera;
	// END GEN INTROSPECTOR
};

std::string get_project_name(const augs::path_type& p);

struct editor_folder {
	editor_folder(const augs::path_type& p = {});

	augs::path_type current_path;

	editor_view view;
	std::unique_ptr<intercosm> work;
	editor_history history;

	/* Opened game mode definitions go here */

	void set_folder_path(sol::state&, const augs::path_type&, editor_recent_paths&);
	std::string get_display_path() const;
	augs::path_type get_autosave_path() const;

	bool is_untitled() const;

	bool has_unsaved_changes() const;

	void save_folder() const;
	void save_folder(const augs::path_type& to) const;
	void save_folder(const augs::path_type& to, const augs::path_type name) const;

	void load_folder();
	void load_folder(const augs::path_type& from);
	void load_folder(const augs::path_type& from, const augs::path_type& name);

	editor_paths get_paths() const;
};

struct editor_last_folders {
	// GEN INTROSPECTOR struct editor_last_folders
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR
};
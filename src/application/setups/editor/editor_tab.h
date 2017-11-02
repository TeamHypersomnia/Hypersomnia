#pragma once
#include <string>
#include <unordered_set>
#include <vector>
#include <optional>

#include "augs/filesystem/path.h"
#include "augs/graphics/vertex.h"
#include "augs/math/camera_cone.h"
#include "game/transcendental/entity_id.h"

namespace sol {
	class state;
}

struct editor_recent_paths;

struct editor_tab {
	// GEN INTROSPECTOR struct editor_tab
	augs::path_type current_path;
	std::unordered_set<entity_id> selected_entities;
	std::optional<camera_cone> editor_mode_cam;
	// END GEN INTROSPECTOR

	void set_workspace_path(sol::state&, const augs::path_type&, editor_recent_paths&);

	bool has_unsaved_changes() const;
	bool is_untitled() const;

	std::string get_display_path() const;
};

struct editor_saved_tabs {
	// GEN INTROSPECTOR struct editor_saved_tabs
	std::size_t current_tab_index = static_cast<std::size_t>(-1);
	std::vector<editor_tab> tabs;
	// END GEN INTROSPECTOR
};
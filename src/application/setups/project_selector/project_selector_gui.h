#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/editor/project/editor_project_meta.h"

namespace augs {
	class window;
}

enum class project_tab_type {
	// GEN INTROSPECTOR enum class project_tab_type
	MY_PROJECTS,
	OFFICIAL_ARENAS,
	DOWNLOADED_ARENAS,

	COUNT
	// END GEN INTROSPECTOR
};

struct project_list_entry { 
	augs::path_type arena_path;
	arena_identifier arena_name;
	ad_hoc_entry_id miniature_index = 0;
	editor_project_about about;
	editor_project_meta meta;

	augs::path_type get_miniature_path() const;
	arena_identifier get_arena_name() const;
};

using project_list_entries = std::vector<project_list_entry>;

struct projects_list_tab_state {
	// GEN INTROSPECTOR struct projects_list_tab_state
	int sort_by_column = 0;
	bool ascending = false;
	// END GEN INTROSPECTOR

	augs::path_type selected_arena_path;
	project_list_entries entries;

	bool perform_list(
		const ad_hoc_in_atlas_map& ad_hoc_atlas,
		std::optional<std::string> timestamp_column_name,
		augs::window& window
	);

	project_list_entry* find_selected();
};

struct projects_list_result {
	augs::path_type opened_project_path;
};

enum class project_list_view_result {
	OPEN_CREATE_DIALOG,
	OPEN_CREATE_FROM_SELECTED_DIALOG,
	OPEN_SELECTED_PROJECT,

	NONE
};

struct projects_list_view {
	// GEN INTROSPECTOR struct projects_list_view
	augs::enum_array<projects_list_tab_state, project_tab_type> tabs;
	project_tab_type current_tab = project_tab_type::MY_PROJECTS;
	// END GEN INTROSPECTOR

	project_list_view_result perform(perform_custom_imgui_input);
	augs::path_type get_selected_project_path() const;
	void select_project(project_tab_type, const augs::path_type&);

	project_list_entry* find_selected();
};

class project_selector_setup;

struct create_new_project_gui : standard_window_mixin<create_new_project_gui> {
	using base = standard_window_mixin<create_new_project_gui>;
	using base::base;
	using introspect_base = base;

	// GEN INTROSPECTOR struct create_new_project_gui
	arena_identifier name;
	arena_short_description_type short_description;

	augs::path_type cloning_from;
	// END GEN INTROSPECTOR

	bool perform(const project_selector_setup&);
};

struct project_selector_gui {
	// GEN INTROSPECTOR struct project_selector_gui
	projects_list_view projects_view;
	create_new_project_gui create_dialog = std::string("New project");
	// END GEN INTROSPECTOR
};

#pragma once

enum class project_tab_type {
	// GEN INTROSPECTOR enum class project_tab_type
	MY_PROJECTS,
	OFFICIAL_TEMPLATES,
	COMMUNITY_ARENAS,

	COUNT
	// END GEN INTROSPECTOR
};

struct project_list_entry { 
	double timestamp;
	augs::path_type arena_path;
	ad_hoc_entry_id miniature_index = 0;
	builder_project_meta meta;

	augs::path_type get_miniature_path() const;
	std::string get_arena_name() const;
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
		const ad_hoc_in_atlas_map& ad_hoc_in_atlas,
		std::optional<std::string> timestamp_column_name
	);

	project_list_entry* find_selected();
};

struct projects_list_result {
	augs::path_type source_project_path;
	augs::path_type target_project_path;
};

struct projects_list_view {
	// GEN INTROSPECTOR struct projects_list_view
	augs::enum_array<projects_list_tab_state, project_tab_type> tabs;
	project_tab_type current_tab = project_tab_type::MY_PROJECTS;
	// END GEN INTROSPECTOR

	std::optional<projects_list_result> perform(perform_custom_imgui_input);
};

struct project_selector_gui {
	// GEN INTROSPECTOR struct project_selector_gui
	projects_list_view projects_view;
	// END GEN INTROSPECTOR
};

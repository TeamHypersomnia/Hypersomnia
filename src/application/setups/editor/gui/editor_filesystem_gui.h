#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/editor_filesystem.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/gui/inspected_variant.h"

class editor_setup;
class images_in_atlas_map;

enum class editor_resources_tab_type {
	PROJECT,
	OFFICIAL
};

struct editor_filesystem_node;
struct editor_history;

namespace augs {
	class window;
}

struct editor_project_files_input {
	editor_setup& setup;
	augs::window& window;
	editor_filesystem_node& project_files_root;
	editor_filesystem_node& official_files_root;
	editor_history& history;

	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
};

struct editor_filesystem_gui : standard_window_mixin<editor_filesystem_gui> {
	using base = standard_window_mixin<editor_filesystem_gui>;
	using base::base;
	using introspect_base = base;

	editor_filesystem_gui(const std::string& name);

	std::optional<inspected_variant> scroll_once_to;
	editor_resources_tab_type current_tab = editor_resources_tab_type::PROJECT;

	editor_resource_id dragged_resource;
	editor_node_id previewed_created_node;
	vec2 world_position_started_dragging;

	entity_id entity_to_highlight;

	void perform(editor_project_files_input);
	void clear_drag_drop();

	bool showing_official() const {
		return current_tab == editor_resources_tab_type::OFFICIAL;
	}

	auto& get_viewed_special_root() {
		return showing_official() ? official_special_root : project_special_root;
	}

	void rebuild_official_special_filesystem(editor_setup&);
	void rebuild_project_special_filesystem(editor_setup&);

private:
	friend editor_setup;
	editor_filesystem_node project_special_root;
	editor_filesystem_node official_special_root;

	void setup_special_filesystem(editor_filesystem_node& root);
	void rebuild_special_filesystem(editor_filesystem_node& root, bool official, editor_setup&);
};


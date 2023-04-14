#pragma once
#include <vector>
#include <string>

#include "augs/templates/type_mod_templates.h"

#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/per_node_type.h"

namespace augs {
	struct introspection_access;
}

struct editor_command_input;

struct clone_nodes_command {
	friend augs::introspection_access;

	struct cloned_entry {
		editor_node_id source_id;
		editor_node_id cloned_id;
	};

	editor_command_meta meta;
private:
	std::vector<cloned_entry> cloned_nodes;
public:
	std::string built_description;
	vec2i mirror_direction;
	std::optional<editor_layer_id> target_new_layer;
	std::optional<std::pair<editor_layer_id, std::size_t>> target_unified_location;
	bool omit_inspector = false;
	std::optional<ltrb> custom_aabb;

	void push_entry(editor_node_id);
	void reverse_order();

	void redo(editor_command_input);
	void undo(editor_command_input);

	auto size() const {
		return cloned_nodes.size();
	}

	bool empty() const;
	std::string describe() const;
	std::vector<editor_node_id> get_all_cloned() const;

	void clear_undo_state();
};

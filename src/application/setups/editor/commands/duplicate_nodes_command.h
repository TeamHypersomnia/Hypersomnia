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

struct duplicate_nodes_command {
	friend augs::introspection_access;

	struct duplicated_entry {
		editor_node_id source_id;
		editor_node_id duplicated_id;
	};

	editor_command_meta meta;
private:
	std::vector<duplicated_entry> duplicated_nodes;
public:
	std::string built_description;
	vec2i mirror_direction;
	std::optional<editor_layer_id> target_new_layer;
	std::optional<std::pair<editor_layer_id, std::size_t>> target_unified_location;
	bool omit_inspector = false;

	void push_entry(editor_node_id);
	void reverse_order();

	void redo(editor_command_input);
	void undo(editor_command_input);

	auto size() const {
		return duplicated_nodes.size();
	}

	bool empty() const;
	std::string describe() const;
	std::vector<editor_node_id> get_all_duplicated() const;

	void clear_undo_state();
};

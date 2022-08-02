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

	template <class E>
	struct duplicated_entry {
		using node_type = E;

		editor_typed_node_id<E> source_id;
		editor_typed_node_id<E> duplicated_id;
	};

	template <class T>
	using make_data_vector = std::vector<duplicated_entry<T>>;

	editor_command_meta meta;
private:
	per_node_type_container<make_data_vector> duplicated_nodes;
public:
	std::string built_description;
	vec2i mirror_direction;
	std::optional<editor_layer_id> target_new_layer;
	bool omit_inspector = false;

	void push_entry(editor_node_id);

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

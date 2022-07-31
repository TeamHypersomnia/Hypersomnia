#pragma once
#include <vector>
#include <string>

#include "augs/misc/pool/pool_structs.h"

#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/nodes/all_editor_node_types.h"
#include "application/setups/editor/nodes/per_node_type.h"

namespace augs {
	struct introspection_access;
}

using editor_node_pool_size_type = unsigned short;
using node_undo_free_input = augs::pool_undo_free_input<editor_node_pool_size_type>;

struct delete_nodes_command {
	friend augs::introspection_access;

	template <class N>
	struct deleted_entry {
		N node_content;
		editor_typed_node_id<N> node_id;
		node_undo_free_input undo_delete_input;
	};

	template <class T>
	using make_data_vector = std::vector<deleted_entry<T>>;

	editor_command_meta meta;
private:
	per_node_type_container<make_data_vector> deleted_nodes;
	editor_layer_pool layers_backup;
public:
	std::string built_description;

	void push_entry(editor_node_id);

	void redo(editor_command_input);
	void undo(editor_command_input);

	auto size() const {
		return deleted_nodes.size();
	}

	bool empty() const;
	std::string describe() const;
};

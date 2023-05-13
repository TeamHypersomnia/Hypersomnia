#pragma once
#include <vector>
#include <string>

#include "augs/misc/pool/pool_structs.h"

#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/resources/all_editor_resource_types.h"
#include "application/setups/editor/resources/per_resource_type.h"

namespace augs {
	struct introspection_access;
}

using editor_resource_pool_size_type = unsigned;
using resource_undo_free_input = augs::pool_undo_free_input<editor_resource_pool_size_type>;

struct delete_resources_command {
	friend augs::introspection_access;

	template <class N>
	struct deleted_entry {
		N resource_content;
		editor_typed_resource_id<N> resource_id;
		resource_undo_free_input undo_delete_input;
	};

	template <class T>
	using make_data_vector = std::vector<deleted_entry<T>>;

	editor_command_meta meta;
private:
	per_internal_resource_type_container<make_data_vector> deleted_resources;
public:
	std::string built_description;
	bool omit_inspector = false;

	void push_entry(editor_resource_id);

	void redo(editor_command_input);
	void undo(editor_command_input);

	auto size() const {
		return deleted_resources.size();
	}

	bool empty() const;
	std::string describe() const;
};

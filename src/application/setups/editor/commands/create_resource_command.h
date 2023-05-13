#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/commands/allocating_command.h"
#include "application/setups/editor/commands/create_layer_command.h"

template <class T>
struct create_resource_command : allocating_command<editor_resource_pool_id> {
	using base = allocating_command<editor_resource_pool_id>;

	std::optional<create_layer_command> create_layer;

	T created_resource;
	editor_layer_id layer_id;
	std::size_t index_in_layer = 0;

	std::string built_description;
	bool omit_inspector = false;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

	editor_resource_id get_resource_id() const;
	editor_typed_resource_id<T> get_typed_resource_id() const;
};

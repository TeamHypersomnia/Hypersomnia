#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

template <class T>
struct edit_resource_command {
	using editable_type = decltype(T::editable);

	editor_typed_resource_id<T> resource_id;

	editor_command_meta meta;

	editable_type before;
	editable_type after;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};

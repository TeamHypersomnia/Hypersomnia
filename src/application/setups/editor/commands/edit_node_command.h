#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"

template <class T>
struct edit_node_command {
	using editable_type = decltype(T::editable);

	editor_command_meta meta;

	struct entry {
		editor_typed_node_id<T> node_id;

		editable_type before;
		editable_type after;
	};

	void push_entry(editor_typed_node_id<T>);

	std::vector<entry> entries;

	auto size() const {
		return entries.size();
	}

	auto empty() const {
		return entries.empty();
	}

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};

struct rename_node_command {
	editor_command_meta meta;

	struct entry {
		editor_node_id node_id;
		std::string before;
	};

	std::vector<entry> entries;

	void push_entry(editor_node_id);

	std::string after;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	auto size() const {
		return entries.size();
	}

	auto empty() const {
		return entries.empty();
	}

	const auto& describe() const {
		return built_description;
	}
};

struct rename_layer_command {
	editor_command_meta meta;

	struct entry {
		editor_layer_id layer_id;
		std::string before;
	};

	std::vector<entry> entries;

	void push_entry(editor_layer_id);

	std::string after;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	auto size() const {
		return entries.size();
	}

	auto empty() const {
		return entries.empty();
	}

	const auto& describe() const {
		return built_description;
	}
};

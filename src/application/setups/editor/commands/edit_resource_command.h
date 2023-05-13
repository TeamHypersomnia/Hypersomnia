#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/gui/inspected_variant.h"

template <class T>
struct edit_resource_command {
	using editable_type = decltype(T::editable);

	editor_command_meta meta;

	struct entry {
		editor_typed_resource_id<T> resource_id;

		editable_type before;
		editable_type after;
	};

	void push_entry(editor_typed_resource_id<T>);

	std::vector<entry> entries;

	auto size() const {
		return entries.size();
	}

	auto empty() const {
		return entries.empty();
	}

	std::string built_description;

	std::optional<std::vector<inspected_variant>> override_inspector_state;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};

struct rename_resource_command {
	editor_command_meta meta;

	struct entry {
		editor_resource_id resource_id;
		std::string before;
	};

	std::vector<entry> entries;

	void push_entry(editor_resource_id);

	template <class T>
	void push_entry(const editor_typed_resource_id<T>& t) {
		push_entry(t.operator editor_resource_id());
	}

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

#pragma once
#include <vector>
#include "augs/templates/traits/component_traits.h"

#include "game/organization/all_components_declaration.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/editor_command_input.h"

template <class derived>
class change_property_command {
	void refresh_other_state(const editor_command_input);

	void rewrite_change_internal(
		const editor_command_input in
	);

public:
	// GEN INTROSPECTOR class change_property_command class derived
	editor_command_common common;

	std::vector<std::byte> values_before_change;
	std::vector<std::byte> value_after_change;

	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;

	template <class T>
	void rewrite_change(
		const T& new_value,
		const editor_command_input in
	);

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

	void clear_undo_state();
};

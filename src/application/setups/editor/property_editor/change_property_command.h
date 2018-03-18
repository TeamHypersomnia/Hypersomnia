#pragma once
#include "augs/callback_result.h"
#include "augs/templates/component_traits.h"
#include "augs/readwrite/byte_readwrite_declaration.h"

#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

#include "application/setups/editor/editor_command_structs.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

template <class T>
static constexpr bool should_reinfer_after_change(const T& invariant) {
	return 
		should_reinfer_when_tweaking_v<T>
		|| is_synchronized_v<T>
	;
}

template <class derived>
class change_property_command {
public:
	// GEN INTROSPECTOR class change_property_command class derived
	editor_command_common common;

	std::vector<std::byte> value_before_change;
	std::vector<std::byte> value_after_change;

	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;

	void rewrite_change(
		const std::vector<std::byte>& new_value,
		const editor_command_input in
	);

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

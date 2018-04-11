#pragma once
#include "augs/enums/callback_result.h"
#include "augs/templates/component_traits.h"

#include "augs/math/transform.h"

#include "game/transcendental/entity_id.h"
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_type_templates.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "application/setups/editor/commands/editor_command_structs.h"

struct editor_command_input;

class move_entities_command {
	template <class T>
	using make_data_vector = std::vector<typed_entity_id<T>>;

public:
	using moved_entities_type = per_entity_type_container<make_data_vector>;
	using delta_type = components::transform;

	// GEN INTROSPECTOR class move_entities_command
	editor_command_common common;
	moved_entities_type moved_entities;

	std::vector<std::byte> values_before_change;

	delta_type move_by;
	std::optional<vec2> rotation_center;

	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;

	void push_entry(const_entity_handle);

	auto size() const {
		return moved_entities.size();
	}

	bool empty() const {
		return size() == 0;
	}

	void rewrite_change(
		const delta_type& new_value,
		std::optional<snapping_grid>,
		const editor_command_input in
	);
	
	void unmove_entities(cosmos& cosm);
	void reinfer_moved(cosmos& cosm);

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

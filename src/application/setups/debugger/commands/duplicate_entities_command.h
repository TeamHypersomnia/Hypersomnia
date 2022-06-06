#pragma once
#include <vector>
#include <string>

#include "augs/templates/type_mod_templates.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/cosmos/per_entity_type.h"
#include "game/cosmos/entity_solvable.h"

#include "application/setups/debugger/commands/debugger_command_structs.h"

namespace augs {
	struct introspection_access;
}

struct duplicate_entities_command {
	friend augs::introspection_access;

	template <class E>
	struct duplicated_entry {
		typed_entity_id<E> source_id;
		typed_entity_id<E> duplicated_id;
	};

	template <class T>
	using make_data_vector = std::vector<duplicated_entry<T>>;

	// GEN INTROSPECTOR struct duplicate_entities_command
	debugger_command_common common;
private:
	per_entity_type_container<make_data_vector> duplicated_entities;
	change_grouping_command created_grouping;
public:
	std::string built_description;
	vec2i mirror_direction;
	// END GEN INTROSPECTOR

	void push_entry(const_entity_handle);

	void redo(debugger_command_input);
	void undo(debugger_command_input);

	auto size() const {
		return duplicated_entities.size();
	}

	bool empty() const;
	std::string describe() const;

	void sanitize(debugger_command_input);
	void clear_undo_state();
};

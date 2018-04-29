#pragma once
#include <vector>
#include <string>

#include "augs/templates/type_mod_templates.h"

#include "augs/misc/pool/pool_structs.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id_declaration.h"
#include "game/transcendental/per_entity_type.h"
#include "game/transcendental/entity_solvable.h"

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_grouping_command.h"

namespace augs {
	struct introspection_access;
}

struct delete_entities_command {
	friend augs::introspection_access;

	template <class E>
	struct deleted_entry {
		entity_solvable<E> content;
		cosmic_pool_undo_free_input undo_delete_input;
	};

	template <class T>
	using make_data_vector = std::vector<deleted_entry<T>>;

	// GEN INTROSPECTOR struct delete_entities_command
	editor_command_common common;
private:
	per_entity_type_container<make_data_vector> deleted_entities;
	change_grouping_command deleted_grouping;
public:
	std::string built_description;
	// END GEN INTROSPECTOR

	void push_entry(const_entity_handle);

	void redo(editor_command_input);
	void undo(editor_command_input);

	auto size() const {
		return deleted_entities.size();
	}

	bool empty() const;
	std::string describe() const;
};

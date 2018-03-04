#pragma once
#include <vector>
#include <variant>

#include "augs/templates/history.h"
#include "augs/templates/type_mod_templates.h"

#include "augs/readwrite/memory_stream.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_type_templates.h"
#include "game/transcendental/entity_solvable.h"

class editor_folder;

struct changed_field_record {
	// GEN INTROSPECTOR struct changed_field_record
	unsigned changed_value_index = 0;

	std::vector<std::byte> new_value;
	std::vector<std::byte> old_value;
	// END GEN INTROSPECTOR
};

struct change_component_command {
	// GEN INTROSPECTOR struct change_component_command
	unsigned component_index = static_cast<unsigned>(-1);
	changed_field_record record;
	// END GEN INTROSPECTOR
};

struct duplicate_entities_command {

};

struct delete_entities_command {
	template <class E>
	struct deleted_entry {
		entity_solvable<E> content;
		cosmic_pool_undo_free_input undo_delete_input;
	};

	template <class T>
	using make_data_vector = std::vector<deleted_entry<T>>;

	// GEN INTROSPECTOR struct delete_entities_command
	per_entity_type<make_data_vector> deleted_entities;
	// END GEN INTROSPECTOR

	void push_entry(const_entity_handle);

	void redo(editor_folder&);
	void undo(editor_folder&) const;

	bool empty() const;
};

struct change_common_state_command {
	// GEN INTROSPECTOR struct change_common_state_command 
	changed_field_record record;
	// END GEN INTROSPECTOR
};

using editor_history = augs::history<
	delete_entities_command
>;

using editor_command = editor_history::command_type;
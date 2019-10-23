#pragma once
#include <vector>
#include <string>

#include "augs/templates/type_mod_templates.h"

#include "augs/misc/pool/pool_structs.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/cosmos/per_entity_type.h"
#include "game/cosmos/entity_solvable.h"

#include "application/setups/editor/commands/editor_command_structs.h"

struct editor_command_input;

namespace augs {
	struct introspection_access;
}

struct paste_entities_command {
	friend augs::introspection_access;

	template <class E>
	struct pasted_entry {
		entity_solvable<E> content;
		entity_id id;
	};

	template <class T>
	using make_data_vector = std::vector<pasted_entry<T>>;

	// GEN INTROSPECTOR struct paste_entities_command
	editor_command_common common;
private:
	per_entity_type_container<make_data_vector> pasted_entities;
public:
	std::string built_description;
	// END GEN INTROSPECTOR

	void push_entry(const_entity_handle);

	void redo(editor_command_input);
	void undo(editor_command_input) const;

	auto size() const {
		return pasted_entities.size();
	}

	bool empty() const;
	std::string describe() const;
};

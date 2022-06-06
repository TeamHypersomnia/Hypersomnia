#pragma once
#include "game/organization/all_entity_types_declaration.h"

#include "application/setups/debugger/editor_command_input.h"
#include "application/setups/debugger/commands/editor_command_structs.h"
#include "application/setups/debugger/commands/id_allocating_command.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"

namespace augs {
	struct introspection_access;
}

struct create_flavour_command : id_allocating_command<raw_entity_flavour_id> {
	using base = id_allocating_command<raw_entity_flavour_id>;
	using introspect_base = base;

	// GEN INTROSPECTOR struct create_flavour_command
	entity_type_id type_id;
private:
	friend augs::introspection_access;
	std::string built_description;
public:
	// END GEN INTROSPECTOR

	std::string describe() const;

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

protected:
	void redo_and_copy(const editor_command_input in, raw_entity_flavour_id);
};

struct duplicate_flavour_command : create_flavour_command {
	using base = create_flavour_command;
	using introspect_base = base;

	// GEN INTROSPECTOR struct duplicate_flavour_command
	raw_entity_flavour_id duplicate_from;
	// END GEN INTROSPECTOR

	duplicate_flavour_command() = default;

	template <class T>
	duplicate_flavour_command(const typed_entity_flavour_id<T>& id) {
		type_id.set<T>();
		duplicate_from = id.raw;
	}

	void redo(const editor_command_input in);
	using base::undo;
};

struct delete_flavour_command : id_freeing_command<raw_entity_flavour_id> {
	using base = id_freeing_command<raw_entity_flavour_id>;
	using introspect_base = base;

	// GEN INTROSPECTOR struct delete_flavour_command
	entity_type_id type_id;
private:
	friend augs::introspection_access;
	std::string built_description;
public:
	// END GEN INTROSPECTOR

	delete_flavour_command() = default;

	template <class T>
	delete_flavour_command(const typed_entity_flavour_id<T>& id) {
		type_id.set<T>();
		freed_id = id.raw;
	}

	std::string describe() const;

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

struct instantiate_flavour_command {
	// GEN INTROSPECTOR struct instantiate_flavour_command
	editor_command_common common;
	entity_flavour_id instantiated_id;
	transformr where;
private:
	friend augs::introspection_access;
	entity_id created_id;
	std::string built_description;
public:
	// END GEN INTROSPECTOR

	auto get_created_id() const {
		return created_id;
	}

	std::string describe() const;

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

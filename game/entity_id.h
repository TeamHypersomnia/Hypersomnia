#pragma once
#include <functional>
#include "game/globals/slot_function.h"
#include "game/globals/associated_entity_name.h"
#include "game/globals/sub_entity_name.h"
#include "game/globals/sub_definition_name.h"

#include "game/globals/sub_definition_name.h"
#include "game/globals/list_of_processing_subjects.h"

#include "game/types_specification/components_instantiation.h"
#include "game/types_specification/full_entity_definition_declaration.h"

struct inventory_slot_id;

namespace components {
	struct relations;
}

class entity_id : public aggregate_id {
	components::relations& relations();
	const components::relations& relations() const;

	void make_child(entity_id p, sub_entity_name);
public:
	inventory_slot_id operator[](slot_function);
	const entity_id& operator[](sub_entity_name) const;
	entity_id& operator[](associated_entity_name);

	full_entity_definition& operator[](sub_definition_name);
	const full_entity_definition& operator[](sub_definition_name) const;

	entity_id get_parent() const;
	bool has(sub_entity_name) const;
	bool has(sub_definition_name) const;
	bool has(associated_entity_name) const;
	bool has(slot_function) const;

	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID);
	void map_sub_entity(sub_entity_name n, entity_id p);
	void map_sub_definition(sub_definition_name n, const full_entity_definition& p);

	void for_each_sub_entity_recursive(std::function<void(entity_id)>);
	void for_each_sub_definition(std::function<void(full_entity_definition&)>);

	void skip_processing_in(list_of_processing_subjects);
	void unskip_processing_in(list_of_processing_subjects);
};

class entity_handle {


};

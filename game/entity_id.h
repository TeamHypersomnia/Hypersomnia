#pragma once
#include <functional>
#include "game/globals/slot_function.h"
#include "game/globals/associated_entity_name.h"
#include "game/globals/sub_entity_name.h"
#include "game/globals/sub_aggregate_name.h"

#include "components_instantiation.h"

struct inventory_slot_id;

namespace components {
	struct relations;
}

class entity_id : public aggregate_id {
	components::relations& get();
	const components::relations& get() const;

public:
	inventory_slot_id operator[](slot_function);
	const entity_id& operator[](sub_entity_name) const;
	const entity_id& operator[](sub_aggregate_name) const;
	entity_id& operator[](associated_entity_name);

	bool has(sub_entity_name) const;
	bool has(sub_aggregate_name) const;
	bool has(associated_entity_name) const;
	bool has(slot_function) const;
};
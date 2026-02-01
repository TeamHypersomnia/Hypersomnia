#pragma once
#include <functional>
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

class cosmos;
struct logic_step;

/* So that we don't have to include the entire cosmic_functions for this simplest task */
entity_handle just_create_entity(
	allocate_new_entity_access access,
	cosmos& cosm,
	const entity_flavour_id id
);

entity_handle just_clone_entity(
	allocate_new_entity_access access,
	const entity_handle source_entity
);

/* Queue a clone request to be flushed later with flush_create_entity_requests */
void queue_clone_entity(
	const logic_step step,
	const entity_id source,
	std::function<void(entity_handle, logic_step)> post_clone
);


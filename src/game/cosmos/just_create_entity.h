#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

class cosmos;

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


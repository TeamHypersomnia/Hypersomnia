#pragma once
#include "game/cosmos/entity_handle_declaration.h"

/* So that we don't have to include the entire cosmic_functions for this simplest task */
entity_handle just_create_entity(
	cosmos& cosm,
	const entity_flavour_id id
);

entity_handle just_clone_entity(const entity_handle source_entity);


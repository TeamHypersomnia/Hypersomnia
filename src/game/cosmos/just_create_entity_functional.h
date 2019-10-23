#pragma once
#include <functional>
#include "game/cosmos/just_create_entity.h"

using pre_construction_callback = std::function<void(entity_handle)>;
using post_construction_callback = pre_construction_callback;

/* So that we don't have to include the entire cosmic_functions for this simplest task */
entity_handle just_create_entity(
	cosmos& cosm,
	const entity_flavour_id id,
	pre_construction_callback pre,
	post_construction_callback post
);

entity_handle just_create_entity(
	cosmos& cosm,
	const entity_flavour_id id,
	pre_construction_callback pre
);

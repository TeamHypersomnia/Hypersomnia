#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"

void release_or_throw_fused_object(
	const logic_step,
	const entity_handle fused_object,
	const entity_id thrower,
	bool is_pressed_flag
);
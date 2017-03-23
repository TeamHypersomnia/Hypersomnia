#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"

void release_or_throw_grenade(
	const logic_step,
	const entity_handle grenade,
	const entity_id thrower,
	bool is_pressed_flag
);
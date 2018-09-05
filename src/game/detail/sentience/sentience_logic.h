#pragma once
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"

struct damage_origin;

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin
);

#pragma once
#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_handle.h"
#include "augs/misc/stepped_timing.h"

namespace components {
	struct sentience;
}

struct spell_logic_input {
	const logic_step step;
	const entity_handle subject;
	components::sentience& sentience;
	const augs::stepped_timestamp when_casted;
	const augs::stepped_timestamp now;
};
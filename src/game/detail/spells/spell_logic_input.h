#pragma once
#include "game/cosmos/logic_step.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/cosmos/entity_handle_declaration.h"

namespace components {
	struct sentience;
}

struct spell_logic_input {
	const logic_step step;
	const entity_id subject;
	components::sentience& sentience;
	const augs::stepped_timestamp when_casted;
	const augs::stepped_timestamp now;

	entity_handle get_subject() const;
};
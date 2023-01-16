#pragma once
#include "game/cosmos/step_declaration.h"

class animation_system {
public:
	void advance_stateful_animations(const logic_step) const;
	void dry_advance_stateful_animations(cosmos& cosm) const;
};
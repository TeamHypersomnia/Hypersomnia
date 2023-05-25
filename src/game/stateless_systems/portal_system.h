#pragma once
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_handle_declaration.h"

class portal_system {
public:
	void advance_portal_logic(const logic_step);
	void finalize_portal_exit(const logic_step, entity_handle teleported, bool reinfer_colliders);
};

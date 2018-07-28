#pragma once
#include "game/cosmos/step_declaration.h"

struct destroy_system {
	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void reverse_perform_deletions(const logic_step);
};
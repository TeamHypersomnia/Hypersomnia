#pragma once
#include "game/transcendental/step_declaration.h"

class destroy_system {
public:
	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void perform_deletions(const logic_step);
};
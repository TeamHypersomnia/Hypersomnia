#pragma once

#include "game/transcendental/step_declaration.h"

class destroy_system {
public:
	void queue_children_of_queued_entities(const logic_step step);
	void perform_deletions(const logic_step);
};
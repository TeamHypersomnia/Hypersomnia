#pragma once
#include "game/cosmos/step_declaration.h"

struct deletion_system {
	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void reverse_perform_deletions(const logic_step);
};

struct allocation_system {
	void flush_pending_allocations(const logic_step);
};
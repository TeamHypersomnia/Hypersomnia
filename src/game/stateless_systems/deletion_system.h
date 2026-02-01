#pragma once
#include "game/cosmos/step_declaration.h"

struct deletion_system {
	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void reverse_perform_deletions(const logic_step);
};

struct creation_system {
	void flush_clone_entity_requests(const logic_step);
	void flush_create_entity_requests(const logic_step);
};
#pragma once
#include <vector>
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/cosmos/step_declaration.h"

struct destroy_system {
	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void reverse_perform_deletions(const logic_step);
};
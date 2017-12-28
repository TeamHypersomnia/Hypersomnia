#pragma once
#include <vector>
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/transcendental/step_declaration.h"

class destroy_system {
public:
	void mark_queued_entities_and_their_children_for_deletion(const destruction_queue&, deletion_queue&, const cosmos& cosm);
	void perform_deletions(const deletion_queue&, cosmos& cosm);

	void mark_queued_entities_and_their_children_for_deletion(const logic_step step);
	void perform_deletions(const logic_step);
};
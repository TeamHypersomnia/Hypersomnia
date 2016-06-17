#include <functional>

#include "destroy_system.h"
#include "game/cosmos.h"
#include "game/entity_id.h"

#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

void destroy_system::queue_children_of_queued_entities() {
	auto& queued = parent_cosmos.messages.get_queue<messages::queue_destruction>();
	auto& deletions = parent_cosmos.messages.get_queue<messages::will_soon_be_deleted>();

	for (auto it : queued) {
		auto deletion_adder = [&deletions](entity_id descendant) {
			deletions.push_back(descendant);
		};

		deletions.push_back(it.subject);
		it.subject.for_each_sub_entity(deletion_adder);
	}

	queued.clear();
}

void destroy_system::perform_deletions() {
	auto& deletions = parent_cosmos.messages.get_queue<messages::will_soon_be_deleted>();

	// destroy in reverse order; children first
	for (auto& it = deletions.rbegin(); it != deletions.rend(); ++it) {
		ensure((*it).subject.alive());

		parent_cosmos.delete_entity((*it).subject);
	}

	deletions.clear();
}

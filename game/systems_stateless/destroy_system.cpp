#include "destroy_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/components/sub_entities_component.h"

#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "augs/ensure.h"

void destroy_system::queue_children_of_queued_entities(const logic_step step) {
	auto& cosmos = step.cosm;
	auto& queued = step.transient.messages.get_queue<messages::queue_destruction>();
	auto& deletions = step.transient.messages.get_queue<messages::will_soon_be_deleted>();

	for (const auto& it : queued) {
		auto deletion_adder = [&deletions](entity_id descendant) {
			deletions.push_back(descendant);
			return true;
		};

		deletions.push_back(it.subject);
		cosmos[it.subject].for_each_sub_entity_recursive(deletion_adder);
	}

	queued.clear();
}

void destroy_system::perform_deletions(const logic_step step) {
	auto& cosmos = step.cosm;
	auto& deletions = step.transient.messages.get_queue<messages::will_soon_be_deleted>();

	// destroy in reverse order; children first
	for (auto& it = deletions.rbegin(); it != deletions.rend(); ++it) {
		// ensure(cosmos[(*it).subject].alive());

		cosmos.delete_entity((*it).subject);
	}

	deletions.clear();
}

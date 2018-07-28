#include "augs/templates/introspect.h"
#include "destroy_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/delete_entity.h"

#include "game/detail/inventory/perform_transfer.h"

void destroy_system::mark_queued_entities_and_their_children_for_deletion(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& queued = step.get_queue<messages::queue_destruction>();
	auto& deletions = step.get_queue<messages::will_soon_be_deleted>();

	make_deletion_queue(queued, deletions, cosmos);
}

void destroy_system::reverse_perform_deletions(const logic_step step) {
	const auto& deletions = step.get_queue<messages::will_soon_be_deleted>();

	::reverse_perform_deletions(deletions, step.get_cosmos());
}

#include "deletion_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/delete_entity.h"
#include "game/cosmos/just_create_entity.h"

#include "game/organization/for_each_entity_type.h"
#include "game/cosmos/create_entity.hpp"
#include "game/messages/clone_entity_message.h"

void deletion_system::mark_queued_entities_and_their_children_for_deletion(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& queued = step.get_queue<messages::queue_deletion>();
	auto& deletions = step.get_queue<messages::will_soon_be_deleted>();

	make_deletion_queue(queued, deletions, cosm);
}

void deletion_system::reverse_perform_deletions(const logic_step step) {
	const auto& deletions = step.get_queue<messages::will_soon_be_deleted>();

	::reverse_perform_deletions(deletions, step.get_cosmos());
}

void creation_system::flush_clone_entity_requests(const logic_step step) {
	auto access = allocate_new_entity_access();
	auto& cosm = step.get_cosmos();
	auto& queued = step.get_queue<messages::clone_entity_message>();

	for (auto& q : queued) {
		const auto source = cosm[q.source];

		if (source.dead()) {
			continue;
		}

		const auto new_entity = just_clone_entity(access, source);

		if (new_entity.alive() && q.post_clone) {
			q.post_clone(new_entity, step);
		}
	}

	queued.clear();
}

void creation_system::flush_create_entity_requests(const logic_step step) {
	/* First flush any queued clone requests */
	flush_clone_entity_requests(step);

	auto access = allocate_new_entity_access();

	auto& cosm = step.get_cosmos();

	for_each_entity_type([&](auto e) {
		using E = decltype(e);

		auto& queued = step.get_queue<messages::create_entity_message<E>>();

		for (auto& q : queued) {
			const ref_typed_entity_handle<E> typed_handle = cosmic::specific_create_entity(
				access,
				cosm, 
				typed_entity_flavour_id<E>(q.flavour_id.raw), 
				[&q](const ref_typed_entity_handle<E> pre_handle, entity_solvable<E>& agg) {
					if (q.pre_construction) {
						q.pre_construction(pre_handle, agg);
					}
				}
			);

			if (q.post_construction) {
				q.post_construction(typed_handle, step);
			}
		}

		queued.clear();
	});
}

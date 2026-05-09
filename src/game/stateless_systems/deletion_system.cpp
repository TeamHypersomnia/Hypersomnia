#include "augs/log.h"
#include "deletion_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/delete_entity.h"
#include "game/cosmos/just_create_entity.h"
#include "game/cosmos/just_create_entity_functional.h"

#include "game/organization/for_each_entity_type.h"
#include "game/cosmos/create_entity.hpp"
#include "game/messages/clone_entity_message.h"
#include "game/messages/just_create_entity_message.h"
#include "game/cosmos/get_corresponding.h"
#include "game/components/interpolation_component.h"

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

	/*
		Move the queue out into a local before iterating, so a post_clone callback
		may safely re-queue more messages without reallocating the vector under us.
		New messages must not be queued from inside post_clone — see post-loop check below.
	*/
	auto local = std::move(queued);
	queued.clear();

	for (auto& q : local) {
		const auto source = cosm[q.source];

		if (source.dead()) {
			continue;
		}

		const auto new_entity = just_clone_entity(access, source);

		if (new_entity.alive() && q.post_clone) {
			q.post_clone(new_entity, step);
		}
	}

	if (!queued.empty()) {
		LOG("ERROR: %x clone_entity_message(s) posted from inside a post_clone callback. Discarding to prevent recursion. Queue allocations outside flush callbacks.", queued.size());
		queued.clear();
	}
}

void creation_system::flush_just_create_entity_requests(const logic_step step) {
	auto access = allocate_new_entity_access();
	auto& cosm = step.get_cosmos();
	auto& queued = step.get_queue<messages::just_create_entity_message>();

	/*
		Move the queue out into a local before iterating, so a post_create callback
		may safely re-queue more messages without reallocating the vector under us.
		New messages must not be queued from inside post_create — see post-loop check below.
	*/
	auto local = std::move(queued);
	queued.clear();

	for (auto& q : local) {
		if (!q.flavour.is_set()) {
			continue;
		}

		const auto new_entity = just_create_entity(access, cosm, q.flavour);

		if (new_entity.alive() && q.post_create) {
			q.post_create(new_entity, step);

			/*
				Re-snap interpolation after post_create moves the entity,
				so it doesn't appear at (0,0) and interpolate towards the real position.
			*/
			new_entity.dispatch([](const auto& typed_handle) {
				if constexpr(remove_cref<decltype(typed_handle)>::template has<invariants::interpolation>()) {
					auto& interp = get_corresponding<components::interpolation>(typed_handle);
					if (const auto t = typed_handle.find_logic_transform()) {
						interp.snap_to(*t);
					}
				}
			});
		}
	}

	if (!queued.empty()) {
		LOG("ERROR: %x just_create_entity_message(s) posted from inside a post_create callback. Discarding to prevent recursion. Queue allocations outside flush callbacks.", queued.size());
		queued.clear();
	}
}

void creation_system::flush_create_entity_requests(const logic_step step) {
	/* First flush any queued clone requests */
	flush_clone_entity_requests(step);

	/* Then flush any queued just_create_entity requests */
	flush_just_create_entity_requests(step);

	auto access = allocate_new_entity_access();

	auto& cosm = step.get_cosmos();

	for_each_entity_type([&](auto e) {
		using E = decltype(e);

		auto& queued = step.get_queue<messages::create_entity_message<E>>();

		/*
			Move the queue out into a local before iterating, so a post_construction
			callback may safely re-queue more messages without reallocating
			the vector under us. New messages must not be queued from inside
			post_construction — see post-loop check below.
		*/
		auto local = std::move(queued);
		queued.clear();

		for (auto& q : local) {
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

		if (!queued.empty()) {
			LOG("ERROR: %x create_entity_message(s) for entity type idx %x posted from inside a post_construction callback. Discarding to prevent recursion. Queue allocations outside flush callbacks.", queued.size(), ENTITY_TYPE_IDX<E>);
			queued.clear();
		}
	});
}

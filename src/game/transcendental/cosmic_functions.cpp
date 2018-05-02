#include "game/transcendental/cosmic_functions.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/inventory/perform_transfer.h"
#include "augs/templates/introspect.h"

void cosmic::clear(cosmos& cosm) {
	cosm.get_solvable({}).clear();
	cosm.change_common_significant([](auto& c) { c = {}; return changer_callback_result::DONT_REFRESH; });
}

void cosmic::infer_caches_for(const entity_handle h) {
	auto& cosm = h.get_cosmos();

	auto constructor = [h](auto, auto& sys) {
		sys.infer_cache_for(h);
	};

	augs::introspect(constructor, cosm.get_solvable_inferred({}));
}

void cosmic::destroy_caches_of(const entity_handle h) {
	auto& cosm = h.get_cosmos();

	auto destructor = [h](auto, auto& sys) {
		sys.destroy_cache_of(h);
	};

	augs::introspect(destructor, cosm.get_solvable_inferred({}));
}

void cosmic::infer_all_entities(cosmos& cosm) {
	for (const auto& ordered_pair : cosm.get_solvable().get_guid_to_id()) {
		infer_caches_for(cosm[ordered_pair.second]);
	}
}

void cosmic::reserve_storage_for_entities(cosmos& cosm, const cosmic_pool_size_type s) {
	cosm.get_solvable({}).reserve_storage_for_entities(s);
}

void cosmic::increment_step(cosmos& cosm) {
	cosm.get_solvable({}).increment_step();
}

void cosmic::reinfer_all_entities(cosmos& cosm) {
	auto scope = measure_scope(cosm.profiler.reinferring_all_entities);

	cosm.get_solvable({}).destroy_all_caches();
	infer_all_entities(cosm);
}

void cosmic::reinfer_solvable(cosmos& cosm) {
	auto& solvable = cosm.get_solvable({});

	solvable.remap_guids();
	reinfer_all_entities(cosm);
}

entity_handle cosmic::clone_entity(const entity_handle source_entity) {
	auto& cosmos = source_entity.get_cosmos();

	if (source_entity.dead()) {
		return entity_handle::dead_handle(cosmos);
	}

	return source_entity.dispatch([](const auto typed_handle){
		return entity_handle(cosmic::specific_clone_entity(typed_handle));
	});
}

#if TODO
entity_handle cosmic::create_entity_with_specific_guid(
	specific_guid_creation_access,
	cosmos& cosm,
   	const entity_guid specific_guid
) {
	return { cosm, cosm.get_solvable({}).allocate_entity_with_specific_guid(specific_guid) };
}
#endif

template <class F>
void entity_deleter(
	const entity_handle handle,
	F actual_deleter
) {
	auto& cosmos = handle.get_cosmos();

	if (handle.dead()) {
		return;
	}

	/* Collect dependent entities so that we might reinfer them */
	std::vector<entity_id> dependent_items;

	handle.dispatch_on_having<invariants::container>([&](const auto typed_handle){
		const auto& container = typed_handle.template get<invariants::container>();

		for (const auto& s : container.slots) {
			concatenate(dependent_items, get_items_inside(typed_handle, s.first));
		}
	});

	/* 
		#2: destroy all associated caches 
		At the moment, all cache classes are designed to be independent.

		There are inter-dependencies inside physics world cache,
		but no top-level cache class in cosmos_solvable_inferred depends on the other.
	*/

	cosmic::destroy_caches_of(handle);

	/* #3: finally, deallocate */
	actual_deleter();

	/* After identity is destroyed, reinfer entities dependent on the identity */
	for (const auto& d : dependent_items) {
		cosmos[d].infer_changed_slot();
	}
}

void cosmic::undo_last_create_entity(const entity_handle handle) {
	entity_deleter(
		handle,
		[&]() {
			handle.get_cosmos().get_solvable({}).undo_last_allocate_entity(handle.get_id());
		}
	);
}

std::optional<cosmic_pool_undo_free_input> cosmic::delete_entity(const entity_handle handle) {
	std::optional<cosmic_pool_undo_free_input> result;
   
	entity_deleter(
		handle,
		[&]() {
			result = handle.get_cosmos().get_solvable({}).free_entity(handle.get_id());
		}
	);

	return result;
}

void delete_entity_with_children(const entity_handle handle) {
	if (handle.dead()) {
		return;
	}

	reverse_perform_deletions(make_deletion_queue(handle), handle.get_cosmos());
}

void make_deletion_queue(
	const const_entity_handle h,
	deletion_queue& q
) {
	q.push_back({ h.get_id() });

	h.for_each_child_entity_recursive([&](const child_entity_id descendant) {
		q.push_back(descendant);
		return callback_result::CONTINUE;
	});
}

void make_deletion_queue(
	const destruction_queue& queued, 
	deletion_queue& deletions, 
	const cosmos& cosmos
) {
	for (const auto& it : queued) {
		make_deletion_queue(cosmos[it.subject], deletions);
	}
}

deletion_queue make_deletion_queue(const const_entity_handle h) {
	thread_local deletion_queue q;
	q.clear();

	q.push_back({ h.get_id() });

	h.for_each_child_entity_recursive([&](const child_entity_id descendant) {
		q.push_back(descendant);
		return callback_result::CONTINUE;
	});

	return q;
}

deletion_queue make_deletion_queue(
	const destruction_queue& queued, 
	const cosmos& cosmos
) {
	thread_local deletion_queue q;
	q.clear();
	make_deletion_queue(queued, q, cosmos);
	return q;
}

void reverse_perform_deletions(const deletion_queue& deletions, cosmos& cosmos) {
	/* 
		The queue is usually populated with entities and their children.
		It makes sense to delete children first, so we iterate it backwards.
	*/

	for (auto it = deletions.rbegin(); it != deletions.rend(); ++it) {
		const auto subject = cosmos[(*it).subject];

		if (subject.dead()) {
			continue;
		}

		cosmic::delete_entity(subject);
	}
}


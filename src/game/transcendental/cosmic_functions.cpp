#include "game/transcendental/cosmic_functions.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/inventory/perform_transfer.h"
#include "augs/templates/introspect.h"

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

	auto& guids = solvable.guid_to_id;
	guids.clear();

	for_each_entity(cosm, [this](const auto handle) {
		guids[handle.get_guid()] = handle.get_id();
	});

	reinfer_all_entities(cosm);
}

entity_handle cosmic::create_entity(
	cosmos& cosm,
   	const entity_flavour_id flavour_id,
   	const components::transform where
) {
	return create_entity(
		cosm, 
		flavour_id, 
		[where](const auto handle) {
			handle.set_logic_transform(where);		
		}
	);
}

entity_handle cosmic::create_entity(
	cosmos& cosm,
   	const entity_flavour_id flavour_id
) {
	return create_entity(
		cosm, 
		flavour_id, 
		[](const auto handle) {}
	);
}

entity_handle cosmic::clone_entity(const const_entity_handle source_entity) {
	if (source_entity.dead()) {
		return entity_handle::dead_handle(cosmos);
	}

	return source_entity.dispatch([](const auto typed_handle){
		using E = entity_type_of<decltype(typed_handle)>;

		return entity_handle(specific_clone_entity<E>(typed_handle));
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


void cosmic::delete_entity(const entity_handle handle) {
	auto& cosmos = handle.get_cosmos();

	if (handle.dead()) {
		return;
	}

	if (const auto container = handle.find<invariants::container>()) {
		drop_from_all_slots(*container, handle, [](const auto&){});
	}

	if (const auto current_slot = handle.get_current_slot()) {
		cosmos.get_solvable_inferred({}).relational.items_of_slots.set_parent(handle, {});
	}

	cosmic::destroy_caches_of(handle);

	cosmos.get_solvable_inferred({}).relational.destroy_caches_of_children_of(handle);
	cosmos.get_solvable({}).free_entity(handle);
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


#include "game/transcendental/cosmic_functions.h"
#include "game/transcendental/cosmos.h"
#include "game/organization/for_each_component_type.h"
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
	cosm.get_solvable({}).remap_guids();
	reinfer_all_entities(cosm);
}

entity_handle cosmic::instantiate_flavour(cosmos& cosm, const entity_flavour_id flavour_id) {
	ensure (flavour_id != entity_flavour_id());

	const auto new_handle = entity_handle { cosm, cosm.get_solvable({}).allocate_next_entity() };

	auto& solvable = cosm.get_solvable({});

	solvable.get_aggregate(new_handle.get_id()).get<components::flavour>(solvable).flavour_id = flavour_id;

	for_each_invariant_type([&](auto d) {
		using D = decltype(d);

		if constexpr(has_implied_component_v<D>) {
			using C = typename D::implied_component;

			const auto& t = new_handle.get_flavour(); 

			if (const auto* const def = t.template find<D>()) {
				new_handle += std::get<C>(t.initial_components);
			}
		}
	});

	if (const auto light = new_handle.find<invariants::light>()) {
		new_handle.add(components::transform());
	}

	return new_handle; 
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

#if TODO
entity_handle cosmic::create_entity_with_specific_guid(
	specific_guid_creation_access,
	cosmos& cosm,
   	const entity_guid specific_guid
) {
	return { cosm, cosm.get_solvable({}).allocate_entity_with_specific_guid(specific_guid) };
}
#endif

entity_handle cosmic::clone_entity(const entity_handle source_entity) {
	auto& cosmos = source_entity.get_cosmos();

	if (source_entity.dead()) {
		return cosmos[entity_id()];
	}

	const auto new_entity = create_entity(cosmos, source_entity.get_flavour_id());
	auto& solvable = cosmos.get_solvable({});

	solvable.get_aggregate(new_entity).clone_components_except<
		/*
			These components will be cloned shortly,
			with due care to each of them.
		*/
		components::guid,
		components::item
	>(solvable.get_aggregate(source_entity), solvable);

#if TODO
	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}
#endif

	{
		for_each_component_type([&](auto c) {
			using component_type = decltype(c);

			if (const auto cloned_to_component = new_entity.get({}).find<component_type>(solvable)) {
				const auto& cloned_from_component = source_entity.get({}).template get<component_type>(solvable);

				if constexpr(allows_nontriviality_v<component_type>) {
					component_type::clone_children(
						cosmos,
						*cloned_to_component, 
						cloned_from_component
					);
				}
				else {
					augs::introspect(
						augs::recursive([&](auto&& self, auto, auto& into, const auto& from) {
							if constexpr(std::is_same_v<decltype(into), child_entity_id&>) {
								into = clone_entity(cosmos[from]);
							}
							else {
								augs::introspect_if_not_leaf(augs::recursive(self), into, from);
							}
						}),
						*cloned_to_component,
						cloned_from_component
					);
				}
			}
		});
	}

	return new_entity;
}

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


#include "game/cosmos/cosmic_functions.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "augs/templates/introspect.h"
#include "game/cosmos/change_common_significant.hpp"
#include "game/cosmos/delete_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/inferred_caches/tree_of_npo_cache.hpp"
#include "game/inferred_caches/organism_cache.hpp"
#include "game/inferred_caches/relational_cache.hpp"
#include "game/inferred_caches/processing_lists_cache.hpp"
#include "game/inferred_caches/flavour_id_cache.hpp"
#include "game/inferred_caches/physics_world_cache.hpp"
#include "game/cosmos/just_create_entity_functional.h"

void cosmic::set_flavour_id_cache_enabled(const bool flag, cosmos& cosm) {
	cosm.get_solvable_inferred({}).flavour_ids.enabled = flag;
}

void cosmic::after_solvable_copy(cosmos& to, const cosmos& from) {
	to.get_solvable_inferred({}).physics.clone_from(from.get_solvable_inferred().physics, to, from);
}

entity_handle just_create_entity(
	allocate_new_entity_access access,
	cosmos& cosm,
	const entity_flavour_id id,
	pre_construction_callback pre,
	post_construction_callback post
) {
	return cosmic::create_entity(access, cosm, id, [&pre](const auto& handle, auto& agg) { pre(handle); (void)agg; }, post);
}

entity_handle just_create_entity(
	allocate_new_entity_access access,
	cosmos& cosm,
	const entity_flavour_id id
) {
	pre_construction_callback pre = [](entity_handle){};
	post_construction_callback post = [](entity_handle){};

	return just_create_entity(access, cosm, id, pre, post);
}

entity_handle just_create_entity(
	allocate_new_entity_access access,
	cosmos& cosm,
	const entity_flavour_id id,
	pre_construction_callback pre
) {
	pre_construction_callback post = [](entity_handle){};

	return just_create_entity(access, cosm, id, pre, post);
}

void cosmic::set_specific_name(const entity_handle& handle, const entity_name_str& name) {
	const auto id = handle.get_id();
	auto& cosm = handle.get_cosmos();
	auto& signi = cosm.get_solvable({}).significant;

	if (name.empty()) {
		erase_element(signi.specific_names, id);
		return;
	}

	signi.specific_names[id] = name;
}

void cosmic::clear(cosmos& cosm) {
	cosm.get_solvable({}).clear();
	
	cosm.change_common_significant([&](cosmos_common_significant& c) {
		augs::introspect(
			[&](auto, auto& field) {
				using T = remove_cref<decltype(field)>;

				if constexpr (std::is_same_v<T, all_logical_assets> || std::is_same_v<T, all_entity_flavours>) {
					field.clear();
				}
				else {
					field = {};
				}
			},
			c
		);

		return changer_callback_result::DONT_REFRESH;
	});
}

void cosmic::infer_caches_for(const entity_handle& in) {
	auto& inferred = in.get_cosmos().get_solvable_inferred({});

	in.dispatch([&](const auto& typed_handle) {
		auto constructor = [&](auto, auto& sys) {
			using S = remove_cref<decltype(sys)>;

			if constexpr(S::template concerned_with<entity_type_of<decltype(typed_handle)>>::value) {
				sys.specific_infer_cache_for(typed_handle);
			}
		};

		augs::introspect(constructor, inferred);
	});
}

void cosmic::destroy_caches_of(const entity_handle& in) {
	auto& inferred = in.get_cosmos().get_solvable_inferred({});

	auto destructor = [&in](auto, auto& sys) {
		sys.destroy_cache_of(in);
	};

	augs::introspect(destructor, inferred);
}

void cosmic::infer_all_entities(cosmos& in) {
	/* 
		Infer domain-wise.

		In normal circumstances, when an entity A depends on caches of entity B,
		the inferrers see all caches constructed for entity B, never just some.

		This works because a dependency of an entity is always fully constructed
		by the time a dependent entity comes into existence.

		Here however, we have no idea which entity depends on which,
		so we solve these dependencies by inferring domain-wise.

		The inferred systems are ordered in such a way that dependencies always go first.
	*/

	auto constructor = [&in](auto, auto& sys) {
		auto& cosm = in;
		sys.infer_all(cosm);
	};

	augs::introspect(constructor, in.get_solvable_inferred({}));
}

void cosmic::reserve_storage_for_entities(cosmos& cosm, const cosmic_pool_size_type s) {
	cosm.get_solvable({}).reserve_storage_for_entities(s);
}

void cosmic::increment_step(cosmos& cosm) {
	cosm.get_solvable({}).increment_step();
}

void cosmic::reinfer_all_entities(cosmos& cosm) {
	LOG("Reinferring all entities at step: %x", cosm.get_timestamp().step);

	auto scope = measure_scope(cosm.profiler.reinferring_all_entities);

	cosm.get_solvable({}).destroy_all_caches();
	infer_all_entities(cosm);
}

void cosmic::reinfer_solvable(cosmos& cosm) {
	reinfer_all_entities(cosm);
}

entity_handle just_clone_entity(
	allocate_new_entity_access access,
	const entity_handle source_entity
) {
	auto& cosm = source_entity.get_cosmos();

	if (source_entity.dead()) {
		return entity_handle::dead_handle(cosm);
	}

	return source_entity.dispatch([access](const auto typed_handle){
		return entity_handle(cosmic::specific_clone_entity(access, typed_handle));
	});
}

template <class F>
void entity_deleter(
	const entity_handle handle,
	F entity_deallocator
) {
	auto& cosm = handle.get_cosmos();

	if (handle.dead()) {
		LOG("WARNING! Trying to delete an entity that is dead already: %x.", handle.get_id());
		return;
	}

	/* Collect dependent entities so that we might reinfer them */
	std::vector<entity_id> dependent_items;

	handle.dispatch_on_having_all<invariants::container>([&](const auto& typed_handle) {
		const auto& container = typed_handle.template get<invariants::container>();

		for (auto&& s : container.slots) {
			concatenate(dependent_items, typed_handle[s.first].get_items_inside());
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
	entity_deallocator();

	/* After identity is destroyed, reinfer caches dependent on some identities */

	for (const auto& d : dependent_items) {
		/* The items that were once assigned to the deleted entity now have no owner */
		cosm[d].infer_change_of_current_slot();
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

void make_deletion_queue(
	const const_entity_handle h,
	deletion_queue& q
) {
	if (h.dead()) {
		LOG("WARNING! Trying to make a deletion queue out of a dead entity.");
		return;
	}

#if LOG_DELETIONS
	LOG("Pushing to deletion queue: %x (root)", h);
#endif

	q.push_back({ h.get_id() });

	h.for_each_child_entity_recursive([&](const child_entity_id descendant) {
		q.push_back(descendant);

#if LOG_DELETIONS
		LOG("Pushing to deletion queue: %x (child)", h.get_cosmos()[descendant]);
#endif

		return callback_result::CONTINUE;
	});
}

void make_deletion_queue(
	const destruction_queue& queued, 
	deletion_queue& deletions, 
	const cosmos& cosm
) {
	for (const auto& it : queued) {
		make_deletion_queue(cosm[it.subject], deletions);
	}
}

deletion_queue make_deletion_queue(const const_entity_handle h) {
	deletion_queue q;
	make_deletion_queue(h, q);
	return q;
}

deletion_queue make_deletion_queue(
	const destruction_queue& queued, 
	const cosmos& cosm
) {
	thread_local deletion_queue q;
	q.clear();
	make_deletion_queue(queued, q, cosm);
	return q;
}

void reverse_perform_deletions(const deletion_queue& deletions, cosmos& cosm) {
	/* 
		The queue is usually populated with entities and their children.
		It makes sense to delete children first, so we iterate it backwards.
	*/

	for (auto it = deletions.rbegin(); it != deletions.rend(); ++it) {
		const auto subject = cosm[(*it).subject];

		if (subject.dead()) {
			continue;
		}

		cosmic::delete_entity(subject);
	}
}

template <class entity_type>
ref_typed_entity_handle<entity_type> cosmic::specific_clone_entity(
	allocate_new_entity_access access,
	const ref_typed_entity_handle<entity_type> in_entity
) {
	const auto source_entity = in_entity.to_const();
	auto& cosm = in_entity.get_cosmos();

	return cosmic::specific_create_entity(access, cosm, source_entity.get_flavour_id(), [&](const auto new_entity, auto&&...) {
		const auto& source_components = source_entity.get({}).component_state;
		auto& new_solvable = new_entity.get({});
		auto& new_components = new_solvable.component_state;

		/* Initial copy-assignment */
		new_components = source_components; 

		cosmic::make_suitable_for_cloning(new_solvable);

		if (const auto slot = source_entity.get_current_slot()) {
			if (const auto source_transform = source_entity.find_logic_transform()) {
				new_entity.set_logic_transform(*source_transform);
			}
		}

		return new_entity;
	});
}

#define INSTANTIATE_SPECIFIC_CLONE(entity_type) template ref_typed_entity_handle<entity_type> cosmic::specific_clone_entity(allocate_new_entity_access access, const ref_typed_entity_handle<entity_type> in_entity);

FOR_ALL_ENTITY_TYPES(INSTANTIATE_SPECIFIC_CLONE)

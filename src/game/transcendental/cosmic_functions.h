#pragma once
#include <tuple>

#include "augs/callback_result.h"
#include "augs/misc/scope_guard.h"

#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/specific_entity_handle_declaration.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/entity_construction.h"

#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/queue_destruction.h"

class cosmic_delta;
class cosmos;

/*
	The purpose of this class is to centralize all functions 
	that can arbitrarily alter the solvable state inside the cosmos,
   	e.g. change fields of synchronized components and then refresh them accordingly.
*/

class cosmic {
	static void destroy_caches_of(const entity_handle h);

	static void infer_all_entities(cosmos& cosm);

	static void reinfer_solvable(cosmos&);

	template <class E>
	static auto instantiate_flavour(
		cosmos& cosm, 
		const typed_entity_flavour_id<F> flavour_id
	) {
		ensure (flavour_id != entity_flavour_id());

		const auto new_allocation = cosm.get_solvable({}).allocate_next_entity<F>(flavour_id);

		const auto result = typed_entity_handle<E> { new_allocation.object, cosm, new_allocation.key };
		new_allocation.object.components = result.get_flavour().initial_components;
		return result;
	}

public:
	class specific_guid_creation_access {
		friend cosmic_delta;

		specific_guid_creation_access() {}
	};

	template <class E, class P>
	static auto create_entity(
		cosmos& cosm, 
		const raw_entity_flavour_id id,
		P pre_construction
	) {
		const auto handle = instantiate_flavour<E>(cosm, id);

		pre_construction(handle);
		construct_entity(handle);
		infer_caches_for(handle);

		return handle;
	}

	template <class P>
	static entity_handle cosmic::create_entity(
		cosmos& cosm,
		const entity_flavour_id flavour_id,
		P&& pre_construction
	) {
		return get_by_dynamic_id(
			all_entity_types(),
			flavour_id.type_id,
			[&](auto e) {
				using E = decltype(e);

				return entity_handle(create_entity<E>(
					cosm, 
					flavour_id, 
					std::forward<P>(pre_construction)
				));
			}
		);
	}

	static entity_handle create_entity(
		cosmos&, 
		entity_flavour_id
	);

	static entity_handle create_entity(
		cosmos&, 
		entity_flavour_id,
		components::transform where
	);

#if TODO
	static entity_handle create_entity_with_specific_guid(
		specific_guid_creation_access,
		cosmos&,
		const entity_guid specific_guid
	);
#endif

	template <class E>
	static auto clone_entity(const const_typed_entity_handle<E> source_entity) {
		auto& cosmos = source_entity.get_cosmos();

		return create_entity<E>(cosmos, source_entity.get_flavour_id(), [&](const auto new_entity) {
			auto& new_components = new_entity.get({}).components;
			new_components = source_entity.get({}).components;

			if (new_components.has<components::item>()) {
				new_components.get<components::item>().current_slot.unset();
			}

			{
				new_components.for_each([&](auto& cloned_to_component) {
					using component_type = std::decay_t<decltype(cloned_to_component);

					const auto& cloned_from_component = source_entity.get({}).template get<component_type>();

					if constexpr(allows_nontriviality_v<component_type>) {
						component_type::clone_children(
							cosmos,
							cloned_to_component, 
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
							cloned_to_component,
							cloned_from_component
						);
					}
				});
			}

			return new_entity;
		});
	}

	static entity_handle clone_entity(const const_entity_handle source_entity);

	static void delete_entity(const entity_handle);

	static void reserve_storage_for_entities(cosmos&, const cosmic_pool_size_type s);
	static void increment_step(cosmos&);

	static void reinfer_all_entities(cosmos&);
	static void infer_caches_for(const entity_handle h);

	template <class C, class F>
	static void change_solvable_significant(C& cosm, F&& callback) {
		auto status = changer_callback_result::INVALID;

		auto refresh_when_done = augs::make_scope_guard([&]() {
			if (status != changer_callback_result::DONT_REFRESH) {
				reinfer_solvable(cosm);
			}
		});

		status = callback(cosm.get_solvable({}).significant);
	}
};

/* Helper functions */

void make_deletion_queue(const const_entity_handle, deletion_queue& q);
void make_deletion_queue(const destruction_queue&, deletion_queue&, const cosmos& cosm);

deletion_queue make_deletion_queue(const const_entity_handle);
deletion_queue make_deletion_queue(const destruction_queue&, const cosmos& cosm);

void reverse_perform_deletions(const deletion_queue&, cosmos& cosm);
void delete_entity_with_children(const entity_handle);


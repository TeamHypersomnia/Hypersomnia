#pragma once
#include <tuple>

#include "augs/callback_result.h"
#include "augs/misc/scope_guard.h"
#include "augs/templates/type_templates.h"

#include "game/transcendental/entity_type_traits.h"
#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/specific_entity_handle.h"
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

	template <class E, class C>
	static auto instantiate_flavour(
		C& cosm, 
		const raw_entity_flavour_id flavour_id
	) {
		ensure (flavour_id != entity_flavour_id());

		const auto new_allocation = cosm.get_solvable({}).template allocate_next_entity<E>(flavour_id);

		const auto result = typed_entity_handle<E> { new_allocation.object, cosm, new_allocation.key };
		new_allocation.object.components = result.get_flavour().initial_components;
		return result;
	}

public:
	class specific_guid_creation_access {
		friend cosmic_delta;

		specific_guid_creation_access() {}
	};

	template <class E, class C, class P>
	static auto specific_create_entity(
		C& cosm, 
		const raw_entity_flavour_id id,
		P pre_construction
	) {
		const auto handle = instantiate_flavour<E>(cosm, id);

		pre_construction(handle);
		construct_pre_inference(handle);
		infer_caches_for(handle);
		construct_post_inference(handle);
		emit_warnings(handle);

		return handle;
	}

	template <class C, class E, class P>
	static typed_entity_handle<E> create_entity(
		C& cosm,
		const typed_entity_flavour_id<E> flavour_id,
		P&& pre_construction
	) {
		return specific_create_entity<E>(cosm, flavour_id.raw, std::forward<P>(pre_construction));
	}

	template <class C, class... Types, class P>
	static entity_handle create_entity(
		C& cosm,
		const constrained_entity_flavour_id<Types...> flavour_id,
		P&& pre_construction
	) {
		using candidate_types = typename decltype(flavour_id)::matching_types; 

		return conditional_get_by_dynamic_id<candidate_types>(
			all_entity_types(),
			flavour_id.type_id,
			[&](auto e) {
				using E = decltype(e);

				return entity_handle(specific_create_entity<E>(
					cosm, 
					flavour_id.raw, 
					std::forward<P>(pre_construction)
				));
			}
		);
	}

#if TODO
	static entity_handle create_entity_with_specific_guid(
		specific_guid_creation_access,
		cosmos&,
		const entity_guid specific_guid
	);
#endif

	template <class E>
	static auto specific_clone_entity(const typed_entity_handle<E> source_entity) {
		auto& cosmos = source_entity.get_cosmos();

		return specific_create_entity<E>(cosmos, source_entity.get_raw_flavour_id(), [&](const auto new_entity) {
			const auto& source_components = source_entity.get({}).components;
			auto& new_components = new_entity.get({}).components;

			/* Initial copy-assignment */
			new_components = source_components; 

			/* Corrections */
			if constexpr(new_entity.template has<components::item>()) {
				std::get<components::item>(new_components).current_slot.unset();
			}

			/* Cloning the children recursively */
#if TODO
			/* TODO: remove child_entity_id from code and introduce groups */
			{
				for_each_through_std_get(new_components, [&](auto& cloned_to_component) {
					using component_type = std::decay_t<decltype(cloned_to_component)>;

					const auto& cloned_from_component = std::get<component_type>(source_components);

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
#endif

			return new_entity;
		});
	}

	static entity_handle clone_entity(const entity_handle source_entity);

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

	template <class... Constraints, class C, class F>
	static void for_each_entity(C& self, F callback) {
		self.get_solvable({}).template for_each_entity<Constraints...>(
			[&](auto& object, const auto iteration_index) {
				using O = decltype(object);
				using E = type_argument_t<std::decay_t<O>>;
				using iterated_handle_type = basic_iterated_entity_handle<is_const_ref_v<O>, E>;
				
				callback(iterated_handle_type(object, self, iteration_index));
			}
		);
	}
};

/* Helper functions */

void make_deletion_queue(const const_entity_handle, deletion_queue& q);
void make_deletion_queue(const destruction_queue&, deletion_queue&, const cosmos& cosm);

deletion_queue make_deletion_queue(const const_entity_handle);
deletion_queue make_deletion_queue(const destruction_queue&, const cosmos& cosm);

void reverse_perform_deletions(const deletion_queue&, cosmos& cosm);
void delete_entity_with_children(const entity_handle);


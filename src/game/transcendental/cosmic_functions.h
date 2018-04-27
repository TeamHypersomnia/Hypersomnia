#pragma once
#include <tuple>

#include "augs/enums/callback_result.h"
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

enum class reinference_type {
	NONE,
	ONLY_AFFECTED,
	ENTIRE_COSMOS
};

class cosmic {
	static void destroy_caches_of(const entity_handle h);

	static void infer_all_entities(cosmos& cosm);

	static void reinfer_solvable(cosmos&);

	template <class F>
	friend void entity_deleter(const entity_handle, F);

	template <class E, class C, class P>
	static auto specific_create_entity_detail(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		const typename entity_flavour<E>::initial_components_type& initial_components,
		P pre_construction
	) {
		const auto new_allocation = cosm.get_solvable({}).template allocate_next_entity<E>(flavour_id.raw);
		const auto handle = ref_typed_entity_handle<E> { cosm, { new_allocation.object, new_allocation.key } };

		{
			auto& object = new_allocation.object;
			object.components = initial_components;
		}

		pre_construction(handle);
		construct_pre_inference(handle);
		infer_caches_for(handle);
		construct_post_inference(handle);
		emit_warnings(handle);

		return handle;
	}

public:
	class specific_guid_creation_access {
		friend cosmic_delta;

		specific_guid_creation_access() {}
	};

	static void clear(cosmos& cosm);

	template <class C, class E>
	static auto specific_paste_entity(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		const typename entity_flavour<E>::initial_components_type& initial_components
	) {
		return specific_create_entity_detail(
			cosm,
		   	flavour_id,
		   	initial_components,
		   	[](auto&&...){}
		);
	}

	template <class C, class E, class P>
	static auto specific_create_entity(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		P&& pre_construction
	) {
		return specific_create_entity_detail(
			cosm,
		   	flavour_id,
		   	cosm.get_flavour(flavour_id).initial_components,
		   	std::forward<P>(pre_construction)
		);
	}

	template <class C, class... Types, class Pre, class Post>
	static entity_handle create_entity(
		C& cosm,
		const constrained_entity_flavour_id<Types...> flavour_id,
		Pre&& pre_construction,
		Post post_construction
	) {
		using candidate_types = typename decltype(flavour_id)::matching_types; 

		return conditional_get_by_dynamic_id<candidate_types>(
			all_entity_types(),
			flavour_id.type_id,
			[&](auto e) {
				using E = decltype(e);

				const auto typed_handle = specific_create_entity(
					cosm, 
					typed_entity_flavour_id<E>(flavour_id.raw), 
					std::forward<Pre>(pre_construction)
				);

				post_construction(typed_handle);
				return entity_handle(typed_handle);
			}
		);
	}

	template <class C, class I, class E>
	static auto undo_delete_entity(
		C& cosm,
		const I undo_delete_input,
		const entity_solvable<E>& deleted_content,
		const reinference_type reinference
	) {
		auto& s = cosm.get_solvable({});

		const auto new_allocation = s.template undo_free_entity<E>(undo_delete_input, deleted_content);
		new_allocation.object.components = deleted_content.components;
		
		const auto handle = ref_typed_entity_handle<E> { cosm, { new_allocation.object, new_allocation.key } };

		if (reinference == reinference_type::ONLY_AFFECTED) {
			infer_caches_for(handle);
		}
		else if (reinference == reinference_type::ENTIRE_COSMOS) {
			reinfer_solvable(cosm);
		}

		return handle;
	}

	template <class E>
	static auto make_suitable_for_cloning(entity_solvable<E>& solvable) {
		auto& new_components = solvable.components;

		if constexpr(entity_solvable<E>::template has<components::item>()) {
			std::get<components::item>(new_components).current_slot.unset();
		}
	}

	template <class handle_type, class P>
	static auto specific_clone_entity(
		const handle_type source_entity,
		P&& pre_construction
	) {
		auto& cosmos = source_entity.get_cosmos();

		return cosmic::specific_create_entity(cosmos, source_entity.get_flavour_id(), [&](const auto new_entity) {
			const auto& source_components = source_entity.get({}).components;
			auto& new_solvable = new_entity.get({});
			auto& new_components = new_solvable.components;

			/* Initial copy-assignment */
			new_components = source_components; 

			cosmic::make_suitable_for_cloning(new_solvable);

			pre_construction(new_entity);

			if (const auto slot = source_entity.get_current_slot()) {
				if (const auto source_transform = source_entity.find_logic_transform()) {
					new_entity.set_logic_transform(*source_transform);
				}
			}

			return new_entity;
		});
	}

	template <class handle_type>
	static auto specific_clone_entity(const handle_type source_entity) {
		return specific_clone_entity(source_entity, [](auto&&...) {});
	}

	static entity_handle clone_entity(const entity_handle source_entity);

	static void undo_last_create_entity(const entity_handle);
	static std::optional<cosmic_pool_undo_free_input> delete_entity(const entity_handle);

	static void reserve_storage_for_entities(cosmos&, const cosmic_pool_size_type s);
	static void increment_step(cosmos&);

	static void reinfer_all_entities(cosmos&);
	static void infer_caches_for(const entity_handle h);

	template <class C, class F>
	static void change_solvable_significant(C& cosm, F&& callback) {
		auto status = changer_callback_result::INVALID;

		auto refresh_when_done = augs::scope_guard([&]() {
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
				using E = typename std::decay_t<O>::used_entity_type;
				using iterated_handle_type = basic_iterated_entity_handle<is_const_ref_v<O>, E>;
				
				callback(iterated_handle_type(self, { object, iteration_index } ));
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


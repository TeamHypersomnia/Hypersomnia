#pragma once
#include "game/cosmos/entity_construction.h"
#include "augs/misc/pool/pool_allocate.h"
#include "game/cosmos/cosmic_functions.h"
#include "game/detail/entity_handle_mixins/get_current_slot.hpp"
#include "game/cosmos/entity_creation_error.h"
#include "game/messages/create_entity_message.h"

template <class E, class C, class I, class P>
ref_typed_entity_handle<E> cosmic::specific_create_entity_detail(
	allocate_new_entity_access access,
	C& cosm, 
	const typed_entity_flavour_id<E> flavour_id,
	const I& initial_components,
	P pre_construction
) {
	static_assert(!std::is_const_v<C>, "Can't create entity in a const cosmos.");

	if (nullptr == cosm.find_flavour(flavour_id)) {
		throw entity_creation_error(entity_creation_error_type::DEAD_FLAVOUR);
	}

	const auto new_allocation = cosm.get_solvable({}).template allocate_next_entity<E>({ access, flavour_id.raw });
	const auto handle = ref_typed_entity_handle<E> { cosm, { new_allocation.object, new_allocation.key } };

	{
		auto& object = new_allocation.object;
		object.component_state = initial_components;
	}

	pre_construction(handle, handle.get({}));
	construct_pre_inference(handle);
	infer_caches_for(handle);
	construct_post_inference(handle);
	emit_warnings(handle);

	return handle;
}

template <class C, class E, class P>
ref_typed_entity_handle<E> cosmic::specific_create_entity(
	allocate_new_entity_access access,
	C& cosm, 
	const typed_entity_flavour_id<E> flavour_id,
	P&& pre_construction
) {
	return specific_create_entity_detail(
		access,
		cosm,
		flavour_id,
		cosm.get_flavour(flavour_id).initial_components,
		std::forward<P>(pre_construction)
	);
}

template <class... Types, class Pre, class Post>
void cosmic::queue_create_entity(
	const logic_step step,
	const constrained_entity_flavour_id<Types...> flavour_id,
	Pre pre_construction,
	Post post_construction
) {
	if (!flavour_id.is_set()) {
		return;
	}

	using candidate_types = typename decltype(flavour_id)::matching_types; 

	constrained_get_by_dynamic_id<candidate_types>(
		all_entity_types(),
		flavour_id.type_id,
		[&](auto e) {
			using E = decltype(e);

			messages::create_entity_message<E> msg;
			msg.flavour_id = typed_entity_flavour_id<E>(flavour_id.raw);

			msg.pre_construction = [pre_construction](ref_typed_entity_handle<E> typed_handle, entity_solvable<E>& agg) {
				pre_construction(typed_handle, agg);
			};

			msg.post_construction = [post_construction](ref_typed_entity_handle<E> typed_handle, const logic_step step) {
				post_construction(typed_handle, step);
			};

			step.post_message(msg);
		}
	);
}

template <class... Types, class Pre>
void cosmic::queue_create_entity(
	const logic_step step,
	const constrained_entity_flavour_id<Types...> flavour_id,
	Pre&& pre_construction
) {
	queue_create_entity(
		step,
		flavour_id,
		std::forward<Pre>(pre_construction),
		[](auto&&...) {}
	);
}

template <class C, class... Types, class Pre, class Post>
entity_handle cosmic::create_entity(
	allocate_new_entity_access access,
	C& cosm,
	const constrained_entity_flavour_id<Types...> flavour_id,
	Pre&& pre_construction,
	Post post_construction
) {
	if (!flavour_id.is_set()) {
		return cosm[entity_id()];
	}

	using candidate_types = typename decltype(flavour_id)::matching_types; 

	return constrained_get_by_dynamic_id<candidate_types>(
		all_entity_types(),
		flavour_id.type_id,
		[&](auto e) {
			using E = decltype(e);

			const auto typed_handle = specific_create_entity(
				access,
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
ref_typed_entity_handle<E> cosmic::undo_delete_entity(
	C& cosm,
	const I undo_delete_input,
	const entity_solvable<E>& deleted_content,
	const reinference_type reinference
) {
	auto& s = cosm.get_solvable({});

	const auto new_allocation = s.template undo_free_entity<E>(undo_delete_input, deleted_content);
	new_allocation.object.component_state = deleted_content.component_state;
	
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
void cosmic::make_suitable_for_cloning(entity_solvable<E>& solvable) {
	auto& new_components = solvable.component_state;

	if constexpr(entity_solvable<E>::template has<components::item>()) {
		std::get<components::item>(new_components).clear_slot_info();
	}
}

template <class E>
auto cosmos_solvable::allocate_new_entity(const entity_creation_input in) {
	auto& pool = significant.get_pool<E>();

	if (pool.full()) {
		throw entity_creation_error(entity_creation_error_type::POOL_FULL);
	}

	const auto result = pool.allocate(in.flavour_id, get_timestamp());

	allocation_result<typed_entity_id<E>, decltype(result.object)> output {
		typed_entity_id<E>(result.key), result.object
	};

	return output;
}

template <class E, class... Args>
auto cosmos_solvable::detail_undo_free_entity(Args&&... args) {
	auto& pool = significant.get_pool<E>();
	const auto result = pool.undo_free(std::forward<Args>(args)...);

	allocation_result<typed_entity_id<E>, decltype(result.object)> output {
		typed_entity_id<E>(result.key), result.object
	};

	return output;
}

template <class E>
auto cosmos_solvable::allocate_next_entity(const entity_creation_input in) {
	return allocate_entity_impl<E>(in);
}

template <class E, class... Args>
auto cosmos_solvable::undo_free_entity(Args&&... undo_free_args) {
	return detail_undo_free_entity<E>(std::forward<Args>(undo_free_args)...);
}

template <class E>
auto cosmos_solvable::allocate_entity_impl(const entity_creation_input in) {
	return allocate_new_entity<E>(in);
}

#pragma once
#include "game/cosmos/entity_construction.h"
#include "game/cosmos/cosmic_functions.h"

template <class E, class C, class I, class P>
ref_typed_entity_handle<E> cosmic::specific_create_entity_detail(
	C& cosm, 
	const typed_entity_flavour_id<E> flavour_id,
	const I& initial_components,
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

template <class C, class I, class E>
ref_typed_entity_handle<E> cosmic::specific_paste_entity(
	C& cosm, 
	const typed_entity_flavour_id<E> flavour_id,
	const I& initial_components
) {
	return specific_create_entity_detail(
		cosm,
		flavour_id,
		initial_components,
		[](auto&&...){}
	);
}

template <class C, class E, class P>
ref_typed_entity_handle<E> cosmic::specific_create_entity(
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
entity_handle cosmic::create_entity(
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
ref_typed_entity_handle<E> cosmic::undo_delete_entity(
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
void cosmic::make_suitable_for_cloning(entity_solvable<E>& solvable) {
	auto& new_components = solvable.components;

	if constexpr(entity_solvable<E>::template has<components::item>()) {
		std::get<components::item>(new_components).current_slot.unset();
	}
}

template <class handle_type, class P>
auto cosmic::specific_clone_entity(
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
auto cosmic::specific_clone_entity(const handle_type source_entity) {
	return specific_clone_entity(source_entity, [](auto&&...) {});
}

template <class E, class... Args>
auto cosmos_solvable::allocate_new_entity(const entity_guid new_guid, Args&&... args) {
	auto& pool = significant.get_pool<E>();

	if (pool.full_capacity()) {
		throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
	}

	const auto result = pool.allocate(new_guid, std::forward<Args>(args)..., get_timestamp());

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

template <class E, class... Args>
auto cosmos_solvable::allocate_next_entity(Args&&... args) {
	const auto next_guid = significant.clock.next_entity_guid.value++;
	return allocate_entity_with_specific_guid<E>(next_guid, std::forward<Args>(args)...);
}

template <class E, class... Args>
auto cosmos_solvable::undo_free_entity(Args&&... undo_free_args) {
	const auto result = detail_undo_free_entity<E>(std::forward<Args>(undo_free_args)...);
	const auto it = guid_to_id.emplace(result.object.guid, result.key);
	ensure(it.second);
	return result;
}

template <class E, class... Args>
auto cosmos_solvable::allocate_entity_with_specific_guid(const entity_guid specific_guid, Args&&... args) {
	const auto result = allocate_new_entity<E>(specific_guid, std::forward<Args>(args)...);
	guid_to_id[specific_guid] = result.key;
	return result;
}

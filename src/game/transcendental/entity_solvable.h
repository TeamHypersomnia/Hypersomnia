#pragma once
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_std_get.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/entity_type.h"

struct entity_solvable_meta {
	// GEN INTROSPECTOR struct entity_solvable_meta
	raw_entity_flavour_id flavour_id;
	entity_guid guid;
	// END GEN INTROSPECTOR

	entity_solvable_meta(
		const raw_entity_flavour_id flavour_id,
		const entity_guid guid
	) :
		flavour_id(flavour_id),
		guid(guid)
	{}
};

template <class E>
struct entity_solvable : entity_solvable_meta {
	using components_type = make_components<E>;

	// GEN INTROSPECTOR struct entity_solvable
	// INTROSPECT BASE entity_solvable_meta
	components_type components;	
	// END GEN INTROSPECTOR

	template <class C>
	constexpr bool has() const {
		return is_one_of_list_v<C, components_type>;
	}

	template <class C>
	auto& get() {
		return std::get<C>(components);
	}

	template <class C>
	const auto& get() const {
		return std::get<C>(components);
	}

	template <class F>
	void for_each(F&& callback) {
		for_each_through_std_get(components, std::forward<F>(callback));
	}

	template <class F>
	void for_each(F&& callback) const {
		for_each_through_std_get(components, std::forward<F>(callback));
	}
};

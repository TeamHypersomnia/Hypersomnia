#pragma once
#include "augs/templates/for_each_std_get.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/entity_type_traits.h"

#include "game/organization/all_entity_types.h"
#include "game/organization/all_component_includes.h"

struct entity_solvable_meta {
	// GEN INTROSPECTOR struct entity_solvable_meta
	raw_entity_flavour_id flavour_id;
	entity_guid guid;
	augs::stepped_timestamp when_born;
	// END GEN INTROSPECTOR

	entity_solvable_meta() = default;

	entity_solvable_meta(
		const entity_guid guid,
		const raw_entity_flavour_id flavour_id,
		const augs::stepped_timestamp when_born
	) :
		flavour_id(flavour_id),
		guid(guid),
		when_born(when_born)
	{}
};

template <class E>
struct entity_solvable : entity_solvable_meta {
	using used_entity_type = E;
	using components_type = make_components<E>;
	using entity_solvable_meta::entity_solvable_meta;
	using introspect_base = entity_solvable_meta;

	// GEN INTROSPECTOR struct entity_solvable class E
	components_type components;	
	// END GEN INTROSPECTOR

	template <class C>
	static constexpr bool has() {
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

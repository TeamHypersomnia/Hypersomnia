#pragma once
#include "augs/templates/for_each_std_get.h"

#include "game/cosmos/entity_flavour_id.h"
#include "game/cosmos/entity_type_traits.h"

#include "game/organization/all_entity_types.h"
#include "game/organization/all_component_includes.h"

struct entity_solvable_meta {
	// GEN INTROSPECTOR struct entity_solvable_meta
	raw_entity_flavour_id flavour_id;
	augs::stepped_timestamp when_born;
	// END GEN INTROSPECTOR

	entity_solvable_meta() = default;

	entity_solvable_meta(
		const raw_entity_flavour_id flavour_id,
		const augs::stepped_timestamp when_born
	) :
		flavour_id(flavour_id),
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
	components_type component_state;	
	// END GEN INTROSPECTOR

	template <class C>
	static constexpr bool has() {
		static_assert(!is_invariant_v<C>, "Don't check for invariants here!");

		return is_one_of_list_v<C, components_type>;
	}

	template <class C>
	auto& get() {
		return std::get<C>(component_state);
	}

	template <class C>
	const auto& get() const {
		return std::get<C>(component_state);
	}

	template <class C>
	C* find() {
		if constexpr(has<C>()) {
			return std::addressof(std::get<C>(component_state));
		}

		return nullptr;
	}

	template <class C>
	const C* find() const {
		if constexpr(has<C>()) {
			return std::addressof(std::get<C>(component_state));
		}

		return nullptr;
	}

	template <class F>
	void for_each(F&& callback) {
		for_each_through_std_get(component_state, std::forward<F>(callback));
	}

	template <class F>
	void for_each(F&& callback) const {
		for_each_through_std_get(component_state, std::forward<F>(callback));
	}
};

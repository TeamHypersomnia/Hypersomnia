#pragma once
#include <tuple>

#include "augs/pad_bytes.h"
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"
#include "game/transcendental/entity_flavour_id.h"
#include "game/organization/all_component_includes.h"

using entity_description_type = entity_name_type;
using entity_initial_components = component_list_t<std::tuple>;
using invariant_tuple = invariant_list_t<augs::trivially_copyable_tuple>;

class entity_flavour {
	template <class D>
	static constexpr auto idx = invariant_index_v<D>;

	template <class D, class E>
	static auto& get_impl(E& self) {
		if constexpr(is_invariant_v<D>) {
			if constexpr(!is_always_present_v<D>) {
				ensure(self.enabled_invariants[idx<D>]); 
			}

			return std::get<D>(self.invariants); 
		}
		else {
			return std::get<D>(self.initial_components); 
		}
	}

	template <class D, class E>
	static auto find_impl(E& self) -> maybe_const_ptr_t<std::is_const_v<E>, D> {
		if constexpr(is_always_present_v<D>) {
			return &get_impl<D>(self);
		}
		else {
			if (self.enabled_invariants[idx<D>]) {
				return std::addressof(std::get<D>(self.invariants));
			}

			return nullptr; 
		}
	}

public:
	// GEN INTROSPECTOR class entity_flavour
	entity_name_type name;
	entity_description_type description;

	std::array<bool, INVARIANTS_COUNT> enabled_invariants = {};
	invariant_tuple invariants;

	entity_initial_components initial_components;
	// END GEN INTROSPECTOR

	template <class D>
	void set(const D& def) {
		if constexpr(is_invariant_v<D>) {
			enabled_invariants[idx<D>] = true;
			std::get<D>(invariants) = def;
		}
		else {
			std::get<D>(initial_components) = def;
		}
	}

	template <class D>
	void remove() {
		if constexpr(is_always_present_v<D>) {
			get<D>() = {};
		}
		else {
			enabled_invariants[idx<D>] = false;
		}
	}

	template <class D>
	D* find() {
		return find_impl<D>(*this);
	}

	template <class D>
	const D* find() const {
		return find_impl<D>(*this);
	}

	template <class D>
	D& get() {
		return get_impl<D>(*this);
	}

	template <class D>
	const D& get() const {
		return get_impl<D>(*this);
	}

	bool operator==(const entity_flavour& b) const {
		return augs::equal_by_introspection(*this, b);
	}

	bool is_set() const {
		return name.size() > 0;
	}

	void unset() {
		name.clear();
	}
};

#if STATICALLY_ALLOCATE_ENTITY_FLAVOURS_NUM
#include "augs/misc/constant_size_vector.h"
using entity_flavours_container = augs::constant_size_vector<entity_flavour, STATICALLY_ALLOCATE_ENTITY_FLAVOURS_NUM>;
#else
#include <vector>
using entity_flavours_container = std::vector<entity_flavour>;
#endif

struct entity_flavours {
	// GEN INTROSPECTOR struct entity_flavours
	entity_flavours_container flavours;
	// END GEN INTROSPECTOR

	auto& get_flavour(const entity_flavour_id id) {
		return flavours[id];
	}

	const auto& get_flavour(const entity_flavour_id id) const {
		return flavours[id];
	}
};
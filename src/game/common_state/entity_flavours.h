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

template <class entity_type>
class entity_flavour {
	using invariants_type = make_invariants<entity_type>;
	using initial_components_type = make_components<entity_type>;

	template <class D, class E>
	static auto& get_impl(E& self) {
		if constexpr(is_invariant_v<D>) {
			return std::get<D>(self.invariants); 
		}
		else {
			return std::get<D>(self.initial_components); 
		}
	}

	template <class D, class E>
	static auto find_impl(E& self) -> maybe_const_ptr_t<std::is_const_v<E>, D> {
		if constexpr(self.has<D>()) {
			return std::addressof(std::get<D>(self.invariants));
		}

		return nullptr; 
	}

public:
	// GEN INTROSPECTOR class entity_flavour
	entity_name_type name;
	entity_description_type description;

	invariants_type invariants;
	initial_components_type initial_components;
	// END GEN INTROSPECTOR

	template <class D>
	void set(const D& def) {
		if constexpr(is_invariant_v<D>) {
			std::get<D>(invariants) = def;
		}
		else {
			std::get<D>(initial_components) = def;
		}
	}

	template <class D>
	constexpr bool has() const {
		return 
			is_one_of_list_v<D, invariants_type>
			|| is_one_of_list_v<D, initial_components_type>
		;
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

#if STATICALLY_ALLOCATE_ENTITY_FLAVOURS
#include "augs/misc/constant_size_vector.h"

template <class E>
using entity_flavours_container = augs::constant_size_vector<
	entity_flavour<E>, 
	E::statically_allocated_flavours
>;

#else
#include <vector>

template <class E>
using entity_flavours_container = std::vector<entity_flavour<E>>;
#endif

template <class entity_type>
struct entity_flavours {
	// GEN INTROSPECTOR struct entity_flavours
	entity_flavours_container<entity_type> flavours;
	// END GEN INTROSPECTOR

	auto& get_flavour(const raw_entity_flavour_id id) {
		return flavours[static_cast<unsigned>(id)];
	}

	const auto& get_flavour(const raw_entity_flavour_id id) const {
		return flavours[static_cast<unsigned>(id)];
	}
};
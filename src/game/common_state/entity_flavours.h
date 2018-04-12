#pragma once
#include <tuple>

#include "augs/pad_bytes.h"
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"
#include "game/transcendental/entity_flavour_id.h"

#include "game/organization/all_component_includes.h"
#include "game/organization/all_entity_types_declaration.h"

template <class entity_type>
class entity_flavour {
	using invariants_type = make_invariants<entity_type>;

	template <class D, class E>
	static auto& get_impl(E& self) {
		if constexpr(is_invariant_v<D>) {
			static_assert(E::template has<D>(), "Entity type does not have this invariant.");
			return std::get<D>(self.invariants); 
		}
		else {
			static_assert(E::template has<D>(), "Unknown initial component type.");
			return std::get<D>(self.initial_components); 
		}
	}

	template <class D, class E>
	static auto find_impl(E& self) -> maybe_const_ptr_t<std::is_const_v<E>, D> {
		if constexpr(E::template has<D>()) {
			return std::addressof(std::get<D>(self.invariants));
		}

		return nullptr; 
	}

public:
	using initial_components_type = make_components<entity_type>;
	using used_entity_type = entity_type;

	// GEN INTROSPECTOR class entity_flavour class entity_type
	invariants_type invariants;
	initial_components_type initial_components;
	// END GEN INTROSPECTOR

	template <class D>
	static constexpr bool has() {
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

	template <class D>
	void set(const D& something) {
		get<D>() = something;
	}

	bool operator==(const entity_flavour& b) const {
		return augs::introspective_equal(*this, b);
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

namespace augs {
	struct introspection_access;
}

template <class entity_type>
class entity_flavours {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct entity_flavours class entity_type
	entity_flavours_container<entity_type> flavours;
	// END GEN INTROSPECTOR

	template <class S, class F>
	static void for_each_impl(S& self, F callback) {
		for (std::size_t i = 0; i < self.count(); ++i) {
			const auto id = raw_entity_flavour_id(static_cast<unsigned>(i));
			callback(typed_entity_flavour_id<entity_type>(id), self.get_flavour(id));
		}
	}

public:
	template <class F>
	void for_each(F&& callback) {
		for_each_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each(F&& callback) const {
		for_each_impl(*this, std::forward<F>(callback));
	}

	void resize(const std::size_t n) {
		flavours.resize(n);
	}

	auto count() const {
		return flavours.size();
	}

	auto& get_flavour(const raw_entity_flavour_id id) {
		return flavours[static_cast<unsigned>(id)];
	}

	const auto& get_flavour(const raw_entity_flavour_id id) const {
		return flavours[static_cast<unsigned>(id)];
	}
};

template <class E>
using make_entity_flavours = entity_flavours<E>;

using all_entity_flavours = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			make_entity_flavours
		>,
		std::tuple
	>
;
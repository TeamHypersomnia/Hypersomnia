#pragma once
#include <tuple>
#include <array>

#include "augs/templates/for_each_std_get.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/container_templates.h"

#include "game/organization/all_entity_types_declaration.h"

template <class E>
struct entity_solvable;

template <template <class> class Mod>
using per_entity_type = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			Mod
		>,
		std::tuple
	>
;

namespace augs {
	struct introspection_access;
}

template <template <class> class Mod>
class per_entity_type_container {
	friend augs::introspection_access;

	// GEN INTROSPECTOR class per_entity_type_container template<class>class Mod
	per_entity_type<Mod> all;
	// END GEN INTROSPECTOR

	template <class S, class F>
	static void for_each_container_impl(S& self, F callback) {
		for_each_through_std_get(
			self.all,
			callback
		);
	}

	template <class S, class F>
	static void for_each_impl(S& self, F callback) {
		for_each_through_std_get(
			self.all,
			[callback](auto& v) {
				for (auto& elem : v) {
					callback(elem);
				}
			}
		);
	}

	template <class S, class F>
	static void for_each_reverse_impl(S& self, F callback) {
		reverse_for_each_through_std_get(
			self.all,
			[callback](auto& v) {
				for (auto& elem : reverse(v)) {
					callback(elem);
				}
			}
		);
	}

public:
	auto size() const {
		std::size_t total = 0;

		for_each_through_std_get(all, [&](const auto& v) {
			total += v.size();
		});

		return total;
	};

	void clear() {
		for_each_through_std_get(all, [&](auto& v) {
			v.clear();
		});
	};

	template <class... Args>
	void reserve(Args&&... args) {
		for_each_through_std_get(all, [&](auto& v) {
			v.reserve(std::forward<Args>(args)...);
		});
	};

	template <class T>
	auto& get() {
		return std::get<T>(all);
	}

	template <class T>
	const auto& get() const {
		return std::get<T>(all);
	}

	template <class E>
	auto& get_for() {
		return std::get<Mod<E>>(all);
	}

	template <class E>
	const auto& get_for() const {
		return std::get<Mod<E>>(all);
	}

	template <class F>
	void for_each_container(F&& callback) {
		for_each_container_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each_container(F&& callback) const {
		for_each_container_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each(F&& callback) {
		for_each_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each(F&& callback) const {
		for_each_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each_reverse(F&& callback) {
		for_each_reverse_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each_reverse(F&& callback) const {
		for_each_reverse_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) visit(const entity_type_id id, F&& callback) {
		return get_by_dynamic_index(all, id.get_index(), std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) visit(const entity_type_id id, F&& callback) const {
		return get_by_dynamic_index(all, id.get_index(), std::forward<F>(callback));
	}
};

template <template <class> class Mod, class F>
void erase_if(per_entity_type_container<Mod>& v, F&& f) {
	v.for_each_container([&f](auto& v) { 
		erase_if(v, std::forward<F>(f));
	});
}

template<template <class> class Mod, template <class> class ElemTemplate, class E>
bool found_in(const per_entity_type_container<Mod>& v, const ElemTemplate<E>& l) {
	return found_in(v.template get_for<E>(), l);
}

template <class T>
using per_entity_type_array = std::array<T, num_types_in_list_v<all_entity_types>>;

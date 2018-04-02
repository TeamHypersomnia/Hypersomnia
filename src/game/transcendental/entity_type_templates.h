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

	template <class T>
	auto& get() {
		return std::get<T>(all);
	}

	template <class T>
	const auto& get() const {
		return std::get<T>(all);
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
};

template <class T>
using per_entity_type_array = std::array<T, num_types_in_list_v<all_entity_types>>;

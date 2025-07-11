#pragma once
#include <cstddef>
#include <tuple>

#include "augs/templates/reversion_wrapper.h"
#include "augs/templates/for_each_std_get.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/get_by_dynamic_id.h"

template <class List, template <class> class Mod>
using per_type_t = 
	replace_list_type_t<
		transform_types_in_list_t<
			List,
			Mod
		>,
		std::tuple
	>
;

namespace augs {
	struct introspection_access;
}

template <class List, template <class> class Mod>
class per_type_container {
	friend augs::introspection_access;

	// GEN INTROSPECTOR class per_type_container class List template<class>class Mod
	per_type_t<List, Mod> all;
	// END GEN INTROSPECTOR

	template <class S, class F>
	static void for_each_container_impl(S& self, F callback) {
		for_each_through_std_get(
			self.all,
			callback
		);
	}

	template <class S, class F>
	static void for_each_container_reverse_impl(S& self, F callback) {
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
	using list_type = List;

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

	template <std::size_t N>
	auto& get_nth() {
		return std::get<N>(all);
	}

	template <std::size_t N>
	const auto& get_nth() const {
		return std::get<N>(all);
	}

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
	decltype(auto) for_each_container(F&& callback) {
		return for_each_container_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_container(F&& callback) const {
		return for_each_container_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_container_reverse(F&& callback) {
		return for_each_container_reverse_impl(*this, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_container_reverse(F&& callback) const {
		return for_each_container_reverse_impl(*this, std::forward<F>(callback));
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

	using id_type = type_in_list_id<List>;

	template <class F>
	decltype(auto) visit(const id_type id, F&& callback) {
		return get_by_dynamic_index(all, id.get_index(), std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) visit(const id_type id, F&& callback) const {
		return get_by_dynamic_index(all, id.get_index(), std::forward<F>(callback));
	}
};

template <class List, template <class> class Mod, class F>
void erase_if(per_type_container<List, Mod>& v, F&& f) {
	v.for_each_container([&f](auto& v) { 
		erase_if(v, std::forward<F>(f));
	});
}

template <class List, template <class> class Mod, template <class> class ElemTemplate, class E>
bool found_in(const per_type_container<List, Mod>& v, const ElemTemplate<E>& l) {
	return found_in(v.template get_for<E>(), l);
}

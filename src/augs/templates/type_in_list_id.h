#pragma once
#include <utility>
#include <compare>
#include "augs/templates/type_list.h"
#include "augs/templates/folded_finders.h"
#include "augs/templates/transform_types.h"
#include "augs/templates/hash_fwd.h"
#include "augs/templates/hash_templates.h"
#include "augs/templates/remove_cref.h"
#include "augs/ensure.h"
#include "augs/ensure_rel.h"

namespace augs {
	struct introspection_access;
}

template <class List>
class type_in_list_id;

template <class T, class F>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
);

template <class OnlyCandidates, class T, class F>
decltype(auto) constrained_get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
);

template <class List>
class type_in_list_id {
public:
	using list_type = replace_list_type_t<List, type_list>;
	using index_type = uint32_t;
private:

	friend augs::introspection_access;
	static constexpr unsigned dead_value = static_cast<unsigned>(-1);
	// GEN INTROSPECTOR class type_in_list_id class L
	index_type index = dead_value;
	// END GEN INTROSPECTOR

	template <class T>
	static void assert_correct_type() {
		static_assert(is_one_of_list_v<T, list_type>, "The type list does not contain the specified type!");
	}
public:

	static constexpr unsigned max_index_v = num_types_in_list_v<list_type>;

	template <class T>
	static auto get_index_of() {
		return static_cast<index_type>(index_in_list_v<T, list_type>);
	}

	type_in_list_id() = default;
	explicit type_in_list_id(const index_type index) : index(index) {}

	template <class T>
	static auto of() {
		return type_in_list_id(get_index_of<T>());
	}

	friend std::ostream& operator<<(std::ostream& out, const type_in_list_id<List> x) {
		return out << x.index;
	}

	bool is_set() const {
		return index < max_index_v;
	}

	void unset() {
		index = dead_value;
	}

	template <class T>
	void set() {
		assert_correct_type<T>();
		index = get_index_of<T>();
	}

	template <class T>
	bool is() const {
		assert_correct_type<T>();
		return index == get_index_of<T>();
	}

	void set_index(const index_type idx) {
		index = idx;
		ensure_less(index, max_index_v);
	}

	index_type get_index() const {
		return index;
	}

	auto operator<=>(const type_in_list_id& b) const = default;

	template <class F>
	decltype(auto) dispatch(F&& callback) const {
		return get_by_dynamic_id(
			List(),
			*this,
			std::forward<F>(callback)
		);
	}

	template <class OnlyCandidates, class F>
	decltype(auto) constrained_dispatch(F&& callback) const {
		return constrained_get_by_dynamic_id<OnlyCandidates>(
			List(),
			*this,
			std::forward<F>(callback)
		);
	}

	auto hash() const {
		return augs::hash_multiple(get_index());
	}
};

namespace std {
	template <class List>
	struct hash<type_in_list_id<List>> {
		std::size_t operator()(const type_in_list_id<List>& k) const {
			return k.hash(); 
		}
	};
}

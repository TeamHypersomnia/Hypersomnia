#pragma once
#include "augs/templates/type_matching_and_indexing.h"

namespace augs {
	struct introspection_access;
}

template <class List>
class type_in_list_id {
	using index_type = unsigned;
	friend struct augs::introspection_access;
	// GEN INTROSPECTOR type_in_list_id class List
	index_type index = 0xdeadbeef;
	// END GEN INTROSPECTOR

	template <class T>
	static auto get_index_of() {
		return static_cast<index_type>(index_in_list_v<T, List>);
	}

	template <class T>
	static void assert_correct_type() {
		static_assert(is_one_of_list_v<T, List>, "The type list does not contain the specified type!");
	}
public:
	bool is_set() const {
		return index != 0xdeadbeef;
	}

	void unset() {
		index = type_in_list_id();
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

	index_type get_index() const {
		return index;
	}

	bool operator==(const type_in_list_id b) const{
		return index == b.index;
	}

	bool operator!=(const type_in_list_id b) const{
		return !operator==(b);
	}
};

namespace std {
	template <class List>
	struct hash<type_in_list_id<List>> {
		std::size_t operator()(const type_in_list_id<List>& k) const {
			return std::hash<decltype(k.get_index())>()(k.get_index());
		}
	};
}
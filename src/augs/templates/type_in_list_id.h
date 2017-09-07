#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_matching_and_indexing.h"

namespace augs {
	struct introspection_access;
}

template <class List>
class type_in_list_id {
	using index_type = unsigned;
	friend struct augs::introspection_access;
	static constexpr std::size_t dead_value = 0xdeadbeef;
	static_assert(num_types_in_list_v<List> < dead_value, "Take it easy, man.");
	// GEN INTROSPECTOR class type_in_list_id class List
	index_type index = dead_value;
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
	type_in_list_id(
		const index_type index = dead_value
	) : index(index) 
	{
	}

	bool is_set() const {
		return index < num_types_in_list_v<List>;
	}

	void unset() {
		*this = type_in_list_id();
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
		ensure(is_set());
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
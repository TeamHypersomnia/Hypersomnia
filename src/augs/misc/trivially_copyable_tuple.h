#pragma once
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/type_list.h"

template <std::size_t I, std::size_t Current, class List, class = void>
struct sum_sizes_until_nth {
	static constexpr std::size_t value = 0;
};

template <std::size_t I, std::size_t Current, template <class...> class List, class T, class... Args>
struct sum_sizes_until_nth<I, Current, List<T, Args...>, std::enable_if_t<Current < I>>  {
	static constexpr std::size_t value = sizeof T + sum_sizes_until_nth<I, Current + 1, List<Args...>>::value;
};

template <std::size_t I, class List>
constexpr std::size_t sum_sizes_until_nth_v = sum_sizes_until_nth<I, 0, List>::value;

template <class List>
constexpr std::size_t sum_sizes_of_types_in_list_v = sum_sizes_until_nth<num_types_in_list_v<List> + 1, 0, List>::value;

namespace augs {
	template <class... Types>
	class trivially_copyable_tuple {
		alignas(constexpr_max_v<std::size_t, alignof(Types)...>) char buf[sum_sizes_of_types_in_list_v<type_list<Types...>>];
	public:
		static_assert(are_types_memcpy_safe_v<Types...>, "One of the tuple types is not trivially copyable!");

		trivially_copyable_tuple() {
			for_each_through_std_get(
				*this,
				[this](auto& field){
					ensure((char*)&field >= (char*)buf && (char*)&field + sizeof (std::decay_t<decltype(field)>) <= buf + sum_sizes_of_types_in_list_v<type_list<Types...>>)
					new (&field) std::decay_t<decltype(field)>;
				}
			);
		}

		char* data() {
			return buf;
		}

		const char* data() const {
			return buf;
		}
	};
}

#include "augs/templates/static_assert_helpers.h"

namespace std {
	template <std::size_t I, class... Types>
	auto& get(augs::trivially_copyable_tuple<Types...>& t) {
		static constexpr std::size_t offset = sum_sizes_until_nth_v<I, type_list<Types...>>;
		using type = nth_type_in_t<I, Types...>;

		static_assert_print<offset < sizeof(t), _types<type>, _vals<std::size_t, I, offset, sizeof(t)>>();
		static_assert_print<offset + sizeof type <= sizeof(t), _types<type>, _vals<std::size_t, I, offset + sizeof type, sizeof(t)>>();

		return *reinterpret_cast<type*>(t.data() + offset);
	}

	template <std::size_t I, class... Types>
	const auto& get(const augs::trivially_copyable_tuple<Types...>& t) {
		static constexpr std::size_t offset = sum_sizes_until_nth_v<I, type_list<Types...>>;
		using type = nth_type_in_t<I, Types...>;

		static_assert_print<offset < sizeof(t), _types<type>, _vals<std::size_t, I, offset, sizeof(t)>>();
		static_assert_print<offset + sizeof type <= sizeof(t), _types<type>, _vals<std::size_t, I, offset + sizeof type, sizeof(t)>>();

		return *reinterpret_cast<const type*>(t.data() + offset);
	}
	
	template<class T, class... Types>
	auto& get(augs::trivially_copyable_tuple<Types...>& t) {
		static_assert(count_occurences_in_v<T, Types...> != 0, "Type not found in the tuple!");
		static_assert(count_occurences_in_v<T, Types...> == 1, "The type occurs in the tuple more than once!");
		return get<index_in_v<T, Types...>>(t);
	}
	
	template<class T, class... Types>
	const auto& get(const augs::trivially_copyable_tuple<Types...>& t) {
		static_assert(count_occurences_in_v<T, Types...> != 0, "Type not found in the tuple!");
		static_assert(count_occurences_in_v<T, Types...> == 1, "The type occurs in the tuple more than once!");
		return get<index_in_v<T, Types...>>(t);
	}
}
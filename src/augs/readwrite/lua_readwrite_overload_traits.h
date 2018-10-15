#pragma once
#include <sol2/sol/forward.hpp>
#include <type_traits>

#define LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED 1

namespace augs {
	template <class ArgsList, class = void>
	struct has_lua_read_overload : std::false_type 
	{};

	template <class... Args>
	struct has_lua_read_overload <
		type_list<Args...>,
		decltype(
			read_object_lua(
				std::declval<sol::table>(),
				std::declval<Args&>()...
			),
			void()
		)
	> : std::true_type 
	{};

	template <class ArgsList, class = void>
	struct has_lua_write_overload : std::false_type 
	{};

	template <class... Args>
	struct has_lua_write_overload <
		type_list<Args...>,
		decltype(
			write_object_lua(
				std::declval<sol::table&>(),
				std::declval<Args&>()...
			),
			void()
		)
	> : std::true_type 
	{};

	template <class... Args>
	constexpr bool has_lua_read_overload_v = has_lua_read_overload<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_lua_write_overload_v = has_lua_write_overload<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_lua_readwrite_overloads_v = 
		has_lua_read_overload_v<Args...> && has_lua_write_overload_v<Args...>
	;
}
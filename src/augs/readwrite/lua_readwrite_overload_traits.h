#pragma once
#include <sol2/sol/forward.hpp>
#include <type_traits>

#define LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED 1

namespace augs {
	class output_stream_reserver;
	class stream;
	
	template <class Serialized, class = void>
	struct has_lua_read_overload : std::false_type 
	{};

	template <class Serialized>
	struct has_lua_read_overload <
		Serialized,
		decltype(
			read_object_lua(
				std::declval<sol::table>(),
				std::declval<Serialized&>()
			),
			void()
		)
	> : std::true_type 
	{};

	template <class Serialized, class = void>
	struct has_lua_write_overload : std::false_type 
	{};

	template <class Serialized>
	struct has_lua_write_overload <
		Serialized,
		decltype(
			write_object_lua(
				std::declval<sol::table>(),
				std::declval<const Serialized&>()
			),
			void()
		)
	> : std::true_type 
	{};

	template <class Serialized>
	constexpr bool has_lua_read_overload_v = has_lua_read_overload<Serialized>::value;

	template <class Serialized>
	constexpr bool has_lua_write_overload_v = has_lua_write_overload<Serialized>::value;

	template <class Serialized>
	constexpr bool has_lua_readwrite_overloads_v = 
		has_lua_read_overload_v<Serialized> && has_lua_write_overload_v<Serialized>
	;
}
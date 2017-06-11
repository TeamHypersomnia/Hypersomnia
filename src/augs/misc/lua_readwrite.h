#pragma once
#include <sol2/sol.hpp>
#include "augs/templates/string_to_enum.h"
#include "augs/misc/templated_readwrite.h"

namespace augs {
	template <class Serialized>
	void read(sol::table input_table, Serialized& into) {
		augs::introspect(
			[input_table](const auto label, auto& field) {
				using T = std::decay_t<decltype(field)>;
				
				if constexpr(!is_padding_field_v<T>) {
					if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
						field = input_table[label].get<T>();
					}
					else if constexpr(std::is_enum_v<T>) {
						if constexpr(has_enum_to_string_v<T>) {
							field = string_to_enum<T>(input_table[label].get<std::string>());
						}
						else {
							field = static_cast<T>(input_table[label].get<int>());
						}
					}
					else {
						static_assert(!is_introspective_leaf_v<T>);

						sol::object maybe_table = input_table[label];
						ensure(maybe_table.is<sol::table>());

						sol::table table = maybe_table;
						read(table, field);
					}
				}
			},
			into
		);
	}

	template <class Serialized>
	void write(sol::table output_table, const Serialized& from) {
		augs::introspect(
			[output_table](const auto label, const auto& field) mutable {
				using T = std::decay_t<decltype(field)>;
				
				if constexpr(!is_padding_field_v<T>) {
					if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
						output_table[label] = field;
					}
					else if constexpr(std::is_enum_v<T>) {
						if constexpr(has_enum_to_string_v<T>) {
							output_table[label] = enum_to_string(field);
						}
						else {
							output_table[label] = static_cast<int>(field);
						}
					}
					else {
						static_assert(!is_introspective_leaf_v<T>);
						write(output_table.create_named(label), field);
					}
				}
			},
			from
		);
	}
}
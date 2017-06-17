#pragma once
#include <sol2/sol.hpp>
#include "augs/templates/string_to_enum.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/misc/custom_lua_representations.h"
#include "augs/misc/script_utils.h"

namespace augs {
	template <class T, class = void>
	struct has_custom_lua_value_representation : std::false_type {};

	template <class T>
	struct has_custom_lua_value_representation<
		T, 
		decltype(
			to_lua_value(
				std::declval<const T>(),
				std::declval<sol::object>()
			),
			from_lua_value(
				std::declval<T>(),
				std::declval<sol::object>()
			),
			void()
		)
	> : std::true_type {};

	template <class T>
	constexpr bool has_custom_lua_value_representation_v = has_custom_lua_value_representation<T>::value;

	template <class T>
	constexpr bool representable_as_lua_value =
		std::is_same_v<T, std::string>
		|| std::is_arithmetic_v<T>
		|| std::is_enum_v<T>
		|| has_custom_lua_value_representation_v<T>
	;

	template <class T, class L>
	void read_table_field(sol::table input_table, T& field, const L label) {
		if constexpr(has_custom_lua_value_representation_v<T>) {
			from_lua_value(field, input_table[label]);
		}
		else if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
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
			static_assert(!representable_as_lua_value<T>, "Non-exhaustive read_table_field");
		}
	}

	template <class T, class L>
	void read_table_or_field(sol::table input_table, T& into, const L label) {
		if constexpr(representable_as_lua_value<T>) {
			read_table_field(input_table, into, label);
		}
		else {
			sol::object maybe_table = input_table[label];
			ensure(maybe_table.is<sol::table>());

			sol::table table = maybe_table;
			read(table, into);
		}
	}

	template <class Serialized>
	void read(sol::table input_table, Serialized& into) {
		static_assert(
			!representable_as_lua_value<Serialized>, 
			"Directly representable, but no key (label) provided! Use read_table_field to directly serialize this object."
		);

		if constexpr(is_container_v<Serialized>) {
			using Container = Serialized;

			int counter = 1;

			while (true) {
				sol::object maybe_element = input_table[counter];

				if (maybe_element.valid()) {
					if constexpr(is_associative_container_v<Container>) {
						typename Container::key_type key;
						typename Container::mapped_type mapped;

						ensure(maybe_element.is<sol::table>());
						
						sol::table key_value_table = maybe_element;

						ensure(key_value_table[1].valid());
						ensure(key_value_table[2].valid());

						read_table_or_field(key_value_table, key, 1);
						read_table_or_field(key_value_table, mapped, 2);

						into.emplace(std::move(key), std::move(mapped));
					}
					else {
						typename Container::value_type val;

						read_table_or_field(input_table, val, counter);

						into.emplace_back(std::move(val));
					}
				}
				else {
					break;
				}

				++counter;
			}
		}
		else {
			augs::introspect(
				[input_table](const auto label, auto& field) {
					using T = std::decay_t<decltype(field)>;
					
					if constexpr(!is_padding_field_v<T>) {
						read_table_or_field(input_table, field, label);
					}
				},
				into
			);
		}
	}


	template <class T, class L>
	void write_table_field(sol::table output_table, const T& field, const L label) {
		if constexpr(has_custom_lua_value_representation_v<T>) {
			to_lua_value(field, output_table[label]);
		}
		else if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
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
			static_assert(!representable_as_lua_value<T>, "Non-exhaustive write_table_field");
		}
	}
	
	template <class T, class L>
	void write_table_or_field(sol::table output_table, const T& from, const L label) {
		if constexpr(representable_as_lua_value<T>) {
			write_table_field(output_table, from, label);
		}
		else {
			auto new_table = output_table.create();
			output_table[label] = new_table;
			write(new_table, from);
		}
	}

	template <class Serialized>
	void write(sol::table output_table, const Serialized& from) {
		static_assert(
			!representable_as_lua_value<Serialized>, 
			"Directly representable, but no key (label) provided! Use write_table_field to directly serialize this object."
		);

		if constexpr(is_container_v<Serialized>) {
			using Container = Serialized;

			int counter = 1;

			for (const auto& element : from) {
				if constexpr(is_associative_container_v<Container>) {
					auto key_value_pair_table = output_table.create();

					write_table_or_field(key_value_pair_table, element.first, 1);
					write_table_or_field(key_value_pair_table, element.second, 2);

					output_table[counter] = key_value_pair_table;
				}
				else {
					write_table_or_field(output_table, element, counter);
				}

				++counter;
			}
		}
		else {
			augs::introspect(
				[output_table](const auto label, const auto& field) mutable {
					using T = std::decay_t<decltype(field)>;
					
					if constexpr(!is_padding_field_v<T>) {
						write_table_or_field(output_table, field, label);
					}
				},
				from
			);
		}
	}

	template <class T, class... SaverArgs>
	void save_as_lua_table(
		T&& object, 
		const std::string& target_path, 
		SaverArgs&&... args
	) {
		auto lua = augs::create_lua_state();

		auto output_table = lua.create_named_table("my_table");
		augs::write(output_table, std::forward<T>(object));

		const std::string file_contents = lua["table_to_string"](
			output_table, 
			std::forward<SaverArgs>(args)...
		);

		augs::create_text_file(target_path, file_contents);
	}

	template <class T>
	void load_from_lua_table(
		T& object,
		const std::string& source_path
	) {
		auto lua = augs::create_lua_state();

		const auto script_contents = "tbl = " + augs::get_file_contents(source_path);
		lua.script(script_contents, augs::lua_error_callback);

		sol::table input_table = lua["tbl"];
		ensure(input_table.valid());

		augs::read(input_table, object);
	}
}
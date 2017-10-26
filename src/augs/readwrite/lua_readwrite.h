#pragma once
#include <sol2/sol.hpp>

#include "augs/pad_bytes.h"

#include "augs/templates/string_to_enum.h"
#include "augs/templates/is_variant.h"
#include "augs/templates/is_optional.h"
#include "augs/templates/exception_templates.h"
#include "augs/templates/introspect.h"
#include "augs/templates/for_each_type.h"

#include "augs/readwrite/custom_lua_representations.h"
#include "augs/misc/lua/lua_utils.h"

#include "augs/filesystem/file.h"

#include "augs/readwrite/readwrite_traits.h"
#include "augs/readwrite/lua_traits.h"

namespace augs {
	struct lua_deserialization_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	template <class T>
	void general_from_lua_value(sol::object object, T& into) {
		if constexpr(std::is_pointer_v<T>) {
			into = nullptr;
		}
		else if constexpr(has_custom_to_lua_value_v<T>) {
			from_lua_value(object, into);
		}
		else if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
			into = object.as<T>();
		}
		else if constexpr(std::is_enum_v<T>) {
			if constexpr(has_enum_to_string_v<T>) {
				const auto stringized_enum = object.as<std::string>();

				if (
					const auto* enumized = mapped_or_nullptr(
						get_string_to_enum_map<T>(), 
						stringized_enum
					)
				) {
					into = *enumized;
				}
				else {
					throw lua_deserialization_error(
						"Failed to read \"%x\" into %x enum. Check if such option exists, or if spelling is correct.",
						stringized_enum,
						typeid(T).name()
					);
				}
			}
			else {
				into = static_cast<T>(object.as<int>());
			}
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive general_from_lua_value");
		}
	}

	template <class Serialized>
	void read(sol::object input_object, Serialized& into) {
		static_assert(
			!is_optional_v<Serialized>,
			"std::optional can only be serialized as a member object."
		);

		if constexpr(has_read_overload_v<sol::table, Serialized>) {
			static_assert(has_write_overload_v<sol::table, Serialized>, "Asymmetric overloads provided for a serialized type");
			
			sol::table input_table = input_object;
			read_object(input_table, into);
		}
		else if constexpr(is_variant_v<Serialized>) {
			sol::table input_table = input_object;
			ensure(input_table.is<sol::table>());

			const auto variant_type_label = get_variant_type_label(into);
			const auto variant_content_label = get_variant_content_label(into);

			std::string variant_type = input_table[variant_type_label];
			sol::object variant_content = input_table[variant_content_label];

			for_each_type_in_list<Serialized>(
				[variant_content, variant_type, &into](auto specific_object){
					const auto this_type_name = get_custom_type_name(specific_object);

					if (this_type_name == variant_type) {
						read(variant_content, specific_object);
						into = specific_object;
					}
				}
			);
		}
		else if constexpr(representable_as_lua_value_v<Serialized>) {
			general_from_lua_value(input_object, into);
		}
		else {
			sol::table input_table = input_object;
			ensure(input_table.is<sol::table>() && "A container must be read from a table, not a value.");
			
			if constexpr(is_container_v<Serialized>) {
				using Container = Serialized;
				
				/*
					If container is associative and the keys are representable as lua values,
					read container table as:
					
					{
						ab = "abc",
						cd = "cde"
					}

					otherwise, read container table as a sequence of key-value pairs (like vector):

					{
						{ "ab", "abc" },
						{ "cd", "cde" }
					}
				*/

				if constexpr(is_associative_v<Container> && key_representable_as_lua_value_v<Container>) {
					for (auto key_value_pair : input_table) {
						typename Container::key_type key{};
						typename Container::mapped_type mapped{};

						general_from_lua_value(key_value_pair.first, key);
						read(key_value_pair.second, mapped);

						into.emplace(std::move(key), std::move(mapped));
					}
				}
				else {
					int counter = 1;

					while (true) {
						sol::object maybe_element = input_table[counter];

						if (maybe_element.valid()) {
							if constexpr(is_associative_v<Container>) {
								typename Container::key_type key{};
								typename Container::mapped_type mapped{};

								ensure(maybe_element.is<sol::table>());
								
								sol::table key_value_table = maybe_element;

								ensure(key_value_table[1].valid());
								ensure(key_value_table[2].valid());

								read(key_value_table[1], key);
								read(key_value_table[2], mapped);

								into.emplace(std::move(key), std::move(mapped));
							}
							else {
								typename Container::value_type val;

								read(input_table[counter], val);

								if constexpr(can_emplace_back_v<Container>) {
									into.emplace_back(std::move(val));
								}
								else {
									into.emplace(std::move(val));
								}
							}
						}
						else {
							break;
						}

						++counter;
					}
				}
			}
			else {
				introspect(
					[input_table](const auto label, auto& field) {
						using T = std::decay_t<decltype(field)>;

						if constexpr(is_optional_v<T>) {
							sol::object maybe_field = input_table[label];
							
							if (maybe_field.valid()) {
								typename T::value_type value;
								read(input_table[label], value);
								field.emplace(std::move(value));
							}
						}
						else if constexpr(!is_padding_field_v<T>) {
							sol::object maybe_field = input_table[label];

							const bool field_specified = maybe_field.valid();
							
							if (field_specified) {
								read(maybe_field, field);
							}
						}
					},
					into
				);
			}
		}
	}

	/*
		For correct overload resolution.
		This prevents the compiler from choosing the templated, byte-wise read
		when the archive type is sol::table - obviously.
	*/

	template <class Serialized>
	void read(sol::table input_table, Serialized& into) {
		read(sol::object(input_table), into);
	}

	template <class A, class B, class Serialized>
	void read(sol::proxy<A, B> input_proxy, Serialized& into) {
		read(sol::object(input_proxy), into);
	}

	template <class T>
	decltype(auto) general_to_lua_value(const T& field) {
		if constexpr(std::is_pointer_v<T>) {
			return general_to_lua_value(*field);
		}
		else if constexpr(has_custom_to_lua_value_v<T>) {
			return to_lua_value(field);
		}
		else if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
			return field;
		}
		else if constexpr(std::is_enum_v<T>) {
			if constexpr(has_enum_to_string_v<T>) {
				return enum_to_string(field);
			}
			else {
				return static_cast<int>(field);
			}
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive to_lua_representation");
		}
	}
	
	template <class T, class K>
	void write_table_or_field(sol::table output_table, const T& from, K&& key) {
		if constexpr(representable_as_lua_value_v<T>) {
			output_table[std::forward<K>(key)] = general_to_lua_value(from);
		}
		else {
			auto new_table = output_table.create();
			output_table[std::forward<K>(key)] = new_table;
			write(new_table, from);
		}
	}

	template <class Serialized>
	void write(sol::table output_table, const Serialized& from) {
		static_assert(
			!representable_as_lua_value_v<Serialized>, 
			"Directly representable, but no key (label) provided! Use write_representable_field to directly serialize this object."
		);

		static_assert(
			!is_optional_v<Serialized>, 
			"std::optional can only be serialized as a member object."
		);

		if constexpr(has_write_overload_v<sol::table, Serialized>) {
			static_assert(has_read_overload_v<sol::table, Serialized>, "Asymmetric overloads provided for a serialized type");

			write_object(output_table, from);
		}
		else if constexpr(is_variant_v<Serialized>) {
			std::visit(
				[output_table](const auto& resolved){
					using T = std::decay_t<decltype(resolved)>;
					
					const auto variant_type_label = get_variant_type_label(std::declval<Serialized&>());
					const auto variant_content_label = get_variant_content_label(std::declval<Serialized&>());
					const auto this_type_name = get_custom_type_name(std::declval<T&>());

					output_table[variant_type_label] = this_type_name;
					write_table_or_field(output_table, resolved, variant_content_label);
				}, 
				from
			);
		}
		else if constexpr(is_container_v<Serialized>) {
			using Container = Serialized;

			/*
				If container is associative and the keys are representable as lua values,
				write container table as:
				
				{
					ab = "abc",
					cd = "cde"
				}

				otherwise, write container table as a sequence of key-value pairs (like vector):

				{
					{ "ab", "abc" },
					{ "cd", "cde" }
				}
			*/

			if constexpr(is_associative_v<Container> && key_representable_as_lua_value_v<Container>) {
				for (const auto& element : from) {
					write_table_or_field(output_table, element.second, general_to_lua_value(element.first));
				}
			}
			else {
				int counter = 1;

				for (const auto& element : from) {
					if constexpr(is_associative_v<Container>) {
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
		}
		else {
			introspect(
				[output_table](const auto label, const auto& field) {
					using T = std::decay_t<decltype(field)>;

					if constexpr(is_optional_v<T>) {
						if (field) {
							write_table_or_field(output_table, field.value(), label);
						}
					}
					else if constexpr(!is_padding_field_v<T>) {
						write_table_or_field(output_table, field, label);
					}
				},
				from
			);
		}
	}

	template <class T, class... SaverArgs>
	void save_as_lua_table(
		sol::state& lua,
		T&& object, 
		const path_type& target_path, 
		SaverArgs&&... args
	) {
		auto output_table = lua.create_named_table("my_table");
		write(output_table, std::forward<T>(object));

		const std::string serialized_table = lua["table_to_string"](
			output_table,
			std::forward<SaverArgs>(args)...
		);

		const auto file_contents = std::string("return ") + serialized_table;
		create_text_file(target_path, file_contents);
	}

	template <class T>
	void load_from_lua_table(
		sol::state& lua,
		T& object,
		const path_type& source_path
	) {
		auto pfr = lua.do_string(
			get_file_contents(source_path)
		);

		if (!pfr.valid()) {
			throw lua_deserialization_error(
				"Failed to obtain table from %x:\n%x",
				source_path,
				pfr.operator std::string()
			);
		}

		sol::table input_table = pfr;
		read(input_table, object);
	}

	template <class T>
	T load_from_lua_table(sol::state& lua, const path_type& source_path) {
		T object;
		load_from_lua_table<T>(lua, object, source_path);
		return object;
	}
}
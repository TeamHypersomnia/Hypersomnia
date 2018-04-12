#pragma once
#include <cstddef>

#include "augs/templates/string_templates.h"
#include "augs/templates/introspect.h"
#include "augs/templates/recursive.h"
#include "augs/templates/can_stream.h"
#include "augs/misc/typesafe_sprintf.h"

template <class T>
std::string conditional_to_string(const T& t) {
	if constexpr(can_stream_left_v<std::ostringstream, T>) {
		std::ostringstream out;
		out << t;
		return out.str();
	}

	return {};
}

template <class T>
auto describe_fields(const T& object) {
	std::string result;
	std::vector<std::string> fields;

	augs::introspect(
		augs::recursive([&](auto&& self, const std::string& label, auto& field) {
			auto make_full_field_name = [&]() {
				std::string name;

				for (const auto& d : fields) {
					name += d + ".";
				}

				return name;
			};

			const auto this_offset = static_cast<std::size_t>(
				reinterpret_cast<const std::byte*>(&field) 
				- reinterpret_cast<const std::byte*>(&object)
			);

			const auto type_name = get_type_name<decltype(field)>();

			result += std::string(fields.size() * 4, ' ') + typesafe_sprintf("%x - %x (%x) (%x) %x",
				this_offset,
				this_offset + sizeof(field),
				sizeof(field),
				// print type name without the leading "struct ", "class " or "enum "
				type_name,
				make_full_field_name() + label
			);

			const auto value = conditional_to_string(field);

			if (value.size() > 0) {
				result += " = " + value + ";";
			}

			result += "\n";

			fields.push_back(label);

			using F = std::decay_t<decltype(field)>;

			if constexpr(!is_container_v<F> && !is_introspective_leaf_v<F>) {
				augs::introspect(augs::recursive(self), field);
			}

			fields.pop_back();
		}), 
		object
	);

	return result;
}

template <class T>
auto determine_breaks_in_fields_continuity_by_introspection(const T& object) {
	std::string result;
	std::vector<std::string> fields;

	std::size_t next_expected_offset = 0;
	std::size_t total_size_of_leaves = 0;

	augs::introspect(
		augs::recursive(
			[&](auto&& self, const std::string& label, auto& field) {
				using F = std::decay_t<decltype(field)>;

				if constexpr(is_container_v<F> || is_introspective_leaf_v<F>) {
					auto make_full_field_name = [&]() {
						std::string name;

						for (const auto& d : fields) {
							name += d + ".";
						}

						return name;
					};

					const auto this_offset = static_cast<std::size_t>(
						reinterpret_cast<const std::byte*>(&field)
						- reinterpret_cast<const std::byte*>(&object)
					);

					if (this_offset != next_expected_offset) {
						const auto type_name = get_type_name<decltype(field)>();

						result += typesafe_sprintf("Field breaks continuity!\nExpected offset: %x\nActual offset: %x\n%x - %x (%x) (%x) %x",
							next_expected_offset,
							this_offset,
							this_offset,
							this_offset + sizeof(field),
							sizeof(field),
							type_name,
							make_full_field_name() + label
						);

						const auto value = conditional_to_string(field);

						if (value.size() > 0) {
							result += " = " + value + ";";
						}

						result += "\n\n";
					}

					next_expected_offset = this_offset + sizeof(field);
					total_size_of_leaves += sizeof(field);
				}
				else {
					fields.push_back(label);
					augs::introspect(augs::recursive(self), field);
					fields.pop_back();
				}
			}
		),
		object
	);

	if (total_size_of_leaves != sizeof(T)) {
		result += typesafe_sprintf(
			"sizeofs of leaf fields do not sum up to sizeof %x!\nExpected: %x\nActual:%x",
			get_type_name<T>(),
			sizeof(T),
			total_size_of_leaves
		);
	}
	
	return result;
}

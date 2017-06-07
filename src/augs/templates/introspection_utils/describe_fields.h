#pragma once
#include "augs/templates/conditional_to_string.h"
#include "augs/templates/string_templates.h"
#include "augs/templates/introspect.h"
#include "augs/misc/typesafe_sprintf.h"

template <class T>
auto describe_fields(const T& object) {
	struct visitor {
		const T& object;
		std::string result;
		std::vector<std::string> fields;

		auto get_visitor() {
			return [this](const std::string& label, auto& field) {
				auto make_full_field_name = [&]() {
					std::string name;

					for (const auto& d : fields) {
						name += d + ".";
					}

					return name;
				};

				const auto this_offset = (char*)&field - (char*)&object;
				auto type_name = std::string(typeid(field).name());
				// print type name without the leading "struct ", "class " or "enum "
				str_ops(type_name).multi_replace_all({ "struct ", "class ", "enum " }, "");

				result += typesafe_sprintf("%x - %x (%x) (%x) %x",
					this_offset,
					this_offset + sizeof field,
					sizeof field,
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
				augs::introspect_if_not_leaf(get_visitor(), field);
				fields.pop_back();
			};
		}
	};

	visitor v { object };
	augs::introspect(v.get_visitor(), object);
	return v.result;
}

template <class T>
auto determine_breaks_in_fields_continuity_by_introspection(const T& object) {
	struct visitor {
		const T& object;
		std::string result;
		std::vector<std::string> fields;

		long next_expected_offset = 0;
		long total_size_of_leaves = 0;

		auto get_visitor() {
			return [&](const std::string& label, auto& field) {
				if constexpr(is_introspective_leaf_v<std::decay_t<decltype(field)>>) {
					auto make_full_field_name = [&]() {
						std::string name;

						for (const auto& d : fields) {
							name += d + ".";
						}

						return name;
					};

					const auto this_offset = (char*)&field - (char*)&object;

					if (this_offset != next_expected_offset) {
						auto type_name = std::string(typeid(field).name());
						// print type name without the leading "struct ", "class " or "enum "
						str_ops(type_name).multi_replace_all({ "struct ", "class ", "enum " }, "");

						result += typesafe_sprintf("Field breaks continuity!\nExpected offset: %x\nActual offset: %x\n%x - %x (%x) (%x) %x",
							next_expected_offset,
							this_offset,
							this_offset,
							this_offset + sizeof field,
							sizeof field,
							type_name,
							make_full_field_name() + label
						);

						const auto value = conditional_to_string(field);

						if (value.size() > 0) {
							result += " = " + value + ";";
						}

						result += "\n\n";
					}

					next_expected_offset = this_offset + sizeof field;
					total_size_of_leaves += sizeof field;
				}
				else {
					fields.push_back(label);
					augs::introspect(get_visitor(), field);
					fields.pop_back();
				}
			};
		}
	};

	visitor v{ object };
	augs::introspect(v.get_visitor(), object);

	if (v.total_size_of_leaves != sizeof T) {
		v.result += typesafe_sprintf(
			"sizeofs of leaf fields do not sum up to sizeof %x!\nExpected: %x\nActual:%x",
			typeid(T).name(),
			sizeof T,
			v.total_size_of_leaves
		);
	}
	
	return v.result;
}

#pragma once
#include "augs/templates/string_templates.h"
#include "augs/templates/introspect.h"
#include "augs/misc/typesafe_sprintf.h"

template <class T>
std::string to_string_or_not(
	const T&, 
	std::enable_if_t<!can_stream_left<std::ostringstream, T>::value>* dummy = nullptr
) {
	return {};
}

template <class T>
std::string to_string_or_not(
	const T& t, 
	std::enable_if_t<can_stream_left<std::ostringstream, T>::value>* dummy = nullptr
) {
	std::ostringstream out;
	out << t;
	return out.str();
}

template <class T>
auto describe_fields(const T& object) {
	std::string result;
	std::vector<std::string> fields;

	auto make_full_field_name = [&fields](){
		std::string name;

		for (const auto& d : fields) {
			name += d + ".";
		}

		return name;
	};

	augs::introspect_recursive_with_prologues<
		true_predicate,
		true_predicate,
		false,
		0
	> (
		[&](const std::string& label, auto& field) {
			const auto this_offset = (char*)&field - (char*)&object;

			result += typesafe_sprintf("%x - %x (%x) (%x) %x",
				this_offset,
				this_offset + sizeof field,
				sizeof field,
				// print type name without the leading "struct ", "class " or "enum "
				replace_all(replace_all(replace_all(std::string(typeid(field).name()), "struct ", ""), "class ", ""), "enum ", ""), 
				make_full_field_name() + label
			);

			const auto value = to_string_or_not(field);

			if (value.size() > 0) {
				result += " = " + value + ";";
			}

			result += "\n";
		},

		[&](const unsigned depth, const std::string& label, auto& field) {
			ensure(fields.size() == depth);
			fields.push_back(label);
		},

		[&](const unsigned depth, const std::string& label, auto...) {
			ensure(fields.back() == label);
			fields.pop_back();
		},

		object
	);

	return result;
}

template <class T>
auto determine_breaks_in_fields_continuity_by_introspection(const T& object) {
	std::string result;
	std::vector<std::string> fields;

	auto make_full_field_name = [&fields](){
		std::string name;

		for (const auto& d : fields) {
			name += d + ".";
		}

		return name;
	};

	long next_expected_offset = 0;
	long total_size_of_leaves = 0;

	augs::introspect_recursive_with_prologues<
		true_predicate,
		true_predicate,
		false,
		0
	> (
		[&](const std::string& label, auto& field) {
			if (is_introspective_leaf_v<std::decay_t<decltype(field)>>) {
				const auto this_offset = (char*)&field - (char*)&object;

				if (this_offset != next_expected_offset) {
					result += typesafe_sprintf("Field breaks continuity!\nExpected offset: %x\nActual offset: %x\n%x - %x (%x) (%x) %x",
						next_expected_offset,
						this_offset,
						this_offset,
						this_offset + sizeof field,
						sizeof field,
						// print type name without the leading "struct ", "class " or "enum "
						replace_all(replace_all(replace_all(std::string(typeid(field).name()), "struct ", ""), "class ", ""), "enum ", ""), 
						make_full_field_name() + label
					);

					const auto value = to_string_or_not(field);

					if (value.size() > 0) {
						result += " = " + value + ";";
					}

					result += "\n\n";
				}

				next_expected_offset = this_offset + sizeof field;
				total_size_of_leaves += sizeof field;
			}
		},

		[&](const unsigned depth, const std::string& label, auto& field) {
			ensure(fields.size() == depth);
			fields.push_back(label);
		},

		[&](const unsigned depth, const std::string& label, auto...) {
			ensure(fields.back() == label);
			fields.pop_back();
		},

		object
	);

	if (total_size_of_leaves != sizeof T) {
		result += typesafe_sprintf(
			"sizeofs of leaf fields do not sum up to sizeof %x!\nExpected: %x\nActual:%x",
			typeid(T).name(),
			sizeof T,
			total_size_of_leaves
		);
	}
	
	return result;
}

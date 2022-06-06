#pragma once
#include <string>

template <class P>
bool is_name_free(
	const P& objects,
	const std::string& tried_name
) {
	for (const auto& o : objects) {
		if (o.get_name() == tried_name) {
			return false;
		}
	}

	return true;
}

template <class P>
std::string find_free_name(
	const P& objects,
	const std::string& basic_name
) {
	auto make_new_name = [&basic_name](const int i) {
		return basic_name + std::to_string(i);
	};

	for (int i = 1; ; ++i) {
		const auto tried_name = make_new_name(i);

		if (is_name_free(objects, tried_name)) {
			return tried_name;
		}
	}
}

template <class P>
std::string find_free_name(
	P& objects,
	const std::string& basic_name,
	const std::string& explicit_suffix
) {
	auto make_new_name = [&basic_name, &explicit_suffix](const int i) {
		if (i == 0) {
			return basic_name;
		}
		
		return basic_name + explicit_suffix + std::to_string(i);
	};

	for (int i = 0; ; ++i) {
		const auto tried_name = make_new_name(i);

		if (is_name_free(objects, tried_name)) {
			return tried_name;
		}
	}
}

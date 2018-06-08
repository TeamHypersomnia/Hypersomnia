#pragma once
#include <string>

template <class P>
std::string find_free_name(
	P& objects,
	const std::string& basic_name
) {
	auto make_new_name = [&basic_name](const int i) {
		return basic_name + std::to_string(i);
	};

	for (int i = 1; ; ++i) {
		const auto tried_name = make_new_name(i);

		bool is_free = true;

		for (const auto& o : objects) {
			if (o.get_name() == tried_name) {
				is_free = false;
				break;
			}
		}

		if (is_free) {
			return tried_name;
		}
	}
}


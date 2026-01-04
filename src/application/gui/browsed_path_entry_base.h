#pragma once
#include "augs/misc/maybe_official_path.h"

std::string& cut_preffix(std::string& value, const std::string& preffix);

template <class id_type>
class browsed_path_entry_base {
	maybe_official_path<id_type> path;
public:

	browsed_path_entry_base() = default;
	browsed_path_entry_base(const maybe_official_path<id_type>& path) : path(path) {}

	bool operator<(const browsed_path_entry_base& b) const {
		return path < b.path;
	}

	auto get_filename() const {
		return path.path.filename();
	}

	auto get_displayed_directory() const {
		return path.suffixed(augs::path_type(path.path).replace_filename("").string());
	}

	const auto& get_full_path() const {
		return path;
	}
};

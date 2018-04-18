#pragma once
#include "augs/filesystem/path_declaration.h"

std::string& cut_preffix(std::string& value, const std::string& preffix);

struct browsed_path_entry_base {
protected:
	augs::path_type p;

public:
	browsed_path_entry_base() = default;
	browsed_path_entry_base(const augs::path_type& p) : p(p) {}

	bool operator<(const browsed_path_entry_base& b) const {
		return p < b.p;
	}

	auto get_filename() const {
		return p.filename();
	}

	auto get_displayed_directory() const {
		auto dir = augs::path_type(p).replace_filename("").string();
		return cut_preffix(dir, "content/");
	}

	const auto& get_full_path() const {
		return p;
	}
};

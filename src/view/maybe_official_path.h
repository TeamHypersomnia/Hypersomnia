#pragma once
#include "augs/filesystem/path.h"

std::string format_field_name(std::string s);

struct maybe_official_path {
	// GEN INTROSPECTOR struct maybe_official_path
	augs::path_type path;
	bool is_official = false;
	// END GEN INTROSPECTOR

	bool operator==(const maybe_official_path& b) const {
		return path == b.path && is_official == b.is_official;
	}

	bool operator<(const maybe_official_path& b) const {
		if (is_official == b.is_official) {
			return path < b.path;
		}

		return (is_official ? 0 : 1) < (b.is_official ? 0 : 1);
	}

	auto suffixed(std::string p) const {
		if (p.size() > 0) {
			p += " ";
		}

		return p + (is_official ? "(Official)" : "(Project)");
	}

	auto to_display() const {
		return suffixed(augs::to_display(path));
	}

	auto to_display_prettified() const {
		return suffixed(augs::to_display_prettified(path));
	}
};


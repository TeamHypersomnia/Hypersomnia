#pragma once
#include "augs/filesystem/path.h"

std::string format_field_name(std::string s);

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

template <class T>
std::string get_content_suffix();

template <class T>
struct maybe_official_path {
	using id_type = T;

	// GEN INTROSPECTOR struct maybe_official_path class T
	augs::path_type path;
	bool is_official = false;
	// END GEN INTROSPECTOR

	bool operator==(const maybe_official_path& b) const {
		return path == b.path && is_official == b.is_official;
	}

	bool operator!=(const maybe_official_path& b) const {
		return !operator==(b);
	}

	bool operator<(const maybe_official_path& b) const {
		if (is_official == b.is_official) {
			return augs::natural_order(path.string(), b.path.string());
		}

		return (is_official ? 1 : 0) < (b.is_official ? 1 : 0);
	}

	auto suffixed(std::string p) const {
		if (p.size() > 0) {
			p += " ";
		}

		return p + (is_official ? "(Official)" : "(Project)");
	}

	auto filename_first() const {
		return suffixed(augs::filename_first(path));
	}

	auto get_prettified_full() const {
		return suffixed(augs::get_prettified_full(path));
	}

	auto get_prettified_filename() const {
		return augs::get_prettified_filename(path);
	}

	static std::string get_content_suffix() {
		return ::get_content_suffix<T>();
	}

	static augs::path_type get_in_official() {
		return augs::path_type(OFFICIAL_CONTENT_DIR) / get_content_suffix();
	}

	auto resolve(const augs::path_type& project_dir) const {
		if (is_official || project_dir.empty()) {
			return get_in_official() / path;
		}

		return project_dir / get_content_suffix() / path;
	}
};

template <class T>
struct is_maybe_official_path : std::false_type {};

template <class T>
struct is_maybe_official_path<maybe_official_path<T>> : std::true_type {};
